#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Math.h>

#include <BleMouse.h>
#include <BleConnectionStatus.h>

#include <BleKeyboard.h>

BleKeyboard bleKeyboard;
BleMouse bleMouse("La manette qui sent drôle", "Emile Maher", 69);

void animateCursor(int mode);

const int potPinX = 34; // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int potPinY = 35; // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int touchButtonPin = 4;

const int button1Pin = 32; // Button 1 is connected to GPIO 32
const int button2Pin = 33; // Button 2 is connected to GPIO 33

const int numberOfPotSamples = 5;        // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 5;       // Delay in milliseconds between pot samples
const int delayBetweenHIDReports = 30; // Additional delay in milliseconds between HID reports

// Declare the MODES array
const String MODES[] = {F("MOUSE"), F("READING")};
int currentModeIndex = 0;

int getNextMode()
{
    int size = sizeof(MODES) / sizeof(MODES[0]);
    String currentMode = MODES[currentModeIndex];
    currentModeIndex = (currentModeIndex + 1) % size;
    return currentModeIndex;
}

void setup()
{
    Serial.begin(9600);
    bleMouse.begin();
}

void animateCursor(int mode) {
    int radius = 10;
    int duration = 500;
    int length = 500;

    if (mode == 0) {
        const int steps = 50;  // Number of steps to complete the circle
        const float angleStep = 2 * PI / steps;  // Angle increment per step
        const int delayPerStep = duration / steps;  // Delay between each step

        for (int i = 0; i <= steps; i++) {
            float angle = i * angleStep;
            int x = static_cast<int>(radius * cos(angle));
            int y = static_cast<int>(radius * sin(angle));
            bleMouse.move(x, y);
            delay(delayPerStep);
        }
    } else if (mode == 1) {
        const int steps = 50;  // Number of steps to complete the line
        const int delayPerStep = duration / steps;  // Delay between each step
        const int stepSize = length / (steps / 2);  // Size of each step

        // Move cursor up
        for (int i = 0; i < steps / 2; i++) {
            bleMouse.move(0, -stepSize);
            delay(delayPerStep);
        }

        // Move cursor down
        for (int i = 0; i < steps / 2; i++) {
            bleMouse.move(0, stepSize);
            delay(delayPerStep);
        }
    }
}

void loop()
{

    if (bleMouse.isConnected())
    {
        int potValuesX[numberOfPotSamples]; // Array to store pot readings
        int potValueX = 0;                  // Variable to store calculated pot reading average
        int potValuesY[numberOfPotSamples]; // Array to store pot readings
        int potValueY = 0;                  // Variable to store calculated pot reading average

        // Populate readings
        for (int i = 0; i < numberOfPotSamples; i++)
        {
            potValuesX[i] = analogRead(potPinX);
            potValueX += potValuesX[i];
            potValuesY[i] = analogRead(potPinY);
            potValueY += potValuesY[i];
            delay(delayBetweenSamples);
        }

        // Calculate the average
        // Calculate the average
        potValueX = static_cast<int>(round(static_cast<double>(potValueX) / numberOfPotSamples));
        potValueY = static_cast<int>(round(static_cast<double>(potValueY) / numberOfPotSamples));
        // Map analog reading from 0 ~ 4095 to 32737 ~ 0 for use as an axis reading
        int offSet = 2000;
        int deadZone = 6000;
        int reductionFactor = 700;
        int maxValue = 32737;

        // Map analog reading from 0 ~ 4095 to 32737 ~ 0 for use as an axis reading
        int adjustedValueX = map(potValueX, 0, 4095, maxValue, 0) - offSet;
        int moveX = int(maxValue / 2 - adjustedValueX);
        if (abs(moveX) < deadZone)
        {
            moveX = 0;
        }
        moveX = moveX / reductionFactor;

        int adjustedValueY = map(potValueY, 0, 4095, maxValue, 0) - offSet;
        int moveY = int(maxValue / 2 - adjustedValueY);
        if (abs(moveY) < deadZone)
        {
            moveY = 0;
        }
        moveY = moveY / reductionFactor;

        int button1Value = analogRead(button1Pin);
        int button2Value = analogRead(button2Pin);
        // Mode 0 : Stick moves cursor, B1 left click
        // Mode 1 : Stick moves scroll/zoom, B1 right click

        if (currentModeIndex == 0 ) {
            bleMouse.move(-moveX, -moveY, 0);
            if (analogRead(button1Pin) > 3000) {
                Serial.println("MOUSE_LEFT");
                bleMouse.click(MOUSE_LEFT);
                delay(500);
            }
        } else if (currentModeIndex == 1 ) {
            // Joystick
            //               ScrollUp
            // Back  ScrollLeft  ScrollRight Forward
            //              ScrollDown
/*
            if (moveX <= -17) {
                Serial.println("MOUSE_BACK");
                bleMouse.click(MOUSE_BACK);
                delay(500);
            } if (moveX >= 17) {
                Serial.println("MOUSE_FORWARD");
                bleMouse.click(MOUSE_FORWARD);
                delay(500);
            } else {
                bleMouse.move(0, 0, 0, round(-moveX / 10));
            }

            if (moveY != 0) {
                bleMouse.move(0, 0, -moveY / abs(moveY));
            }
*/
            bleMouse.move(0, 0, round(moveY / 20), round(moveX / 20));

            if (analogRead(button1Pin) > 3000) {
                Serial.println("MOUSE_RIGHT");
                bleMouse.click(MOUSE_RIGHT);
                delay(500);
            }
        }

        // Button 2 changes mode
        if (analogRead(button2Pin) > 3000) {
            currentModeIndex = getNextMode();
            Serial.println("Button 2 pressed : Mode set to " + String(currentModeIndex));
            
            animateCursor(currentModeIndex);
        }

        // Debug
        if (moveX != 0 || moveY != 0) {
            Serial.println("adjustedValueX : " + String(adjustedValueX) + " moveX : " + String(moveX));
            Serial.println("adjustedValueY : " + String(adjustedValueY) + " moveY : " + String(moveY));
        }
        /*
        Serial.println("button 1 : " + String(button1Value));
        Serial.println("button 2 : " + String(button2Value));
        */

        delay(delayBetweenHIDReports); // bluetooth stack will go into congestion, if too many packets are sent
    }
}
