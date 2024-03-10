#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <BleMouse.h>
#include <BleConnectionStatus.h>

const int potPinX = 34; // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int potPinY = 35; // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int touchButtonPin = 4;
const int buttonMode = 34;
const int button1Pin = 32; // Button 1 is connected to GPIO 32
const int button2Pin = 33; // Button 2 is connected to GPIO 33

const int numberOfPotSamples = 5;        // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 5;       // Delay in milliseconds between pot samples
const int delayBetweenHIDReports = 1000; // Additional delay in milliseconds between HID reports

BleMouse bleMouse("La manette qui sent drôle", "Emile Maher", 69);

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

void loop()
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
    int offSet = 1000;
    int deadZone = 4000;
    int reductionFactor = 900;
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


    if (bleMouse.isConnected())
    {
        Serial.println("connected");
        // bleMouse.move(-moveX, -moveY, 0);
        moveY = 1;
        Serial.println("adjustedValueX : " + String(adjustedValueX) + " moveX : " + String(moveX));
        Serial.println("adjustedValueY : " + String(adjustedValueY) + " moveY : " + String(moveY));
        bleMouse.move(moveX, moveY, 0);
        /*
                if (touchRead(touchButtonPin) < 30) {
                    bleMouse.press(MOUSE_LEFT);
                    Serial.println("Left button pressed");
                } else {
                    bleMouse.release(MOUSE_LEFT);
                }

                if (analogRead(buttonMode) < 30) {
                    Serial.println("Mode button pressed");
                    currentModeIndex = getNextMode();
                }
        */
        delay(delayBetweenHIDReports); // bluetooth stack will go into congestion, if too many packets are sent
    }
}
