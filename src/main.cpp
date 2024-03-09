#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <BleMouse.h>
#include <BleConnectionStatus.h>

const int potPinX = 34;                // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int potPinY = 35;                // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int touchButtonPin = 4;     
const int buttonMode = 666;

const int numberOfPotSamples = 5;     // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 5;    // Delay in milliseconds between pot samples
const int delayBetweenHIDReports = 100; // Additional delay in milliseconds between HID reports

BleMouse bleMouse("La manette qui sent drôle", "Emile Maher", 69);

const String a = "22";
const String MODES[] = {"MOUSE", "READING"};

int modeJoystick = 0;

void setup() {
    Serial.begin(9600);
    bleMouse.begin();
}

void loop() {
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
    potValueX = (potValueX / numberOfPotSamples).toInt();
    potValueY = (potValueY / numberOfPotSamples).toInt();

    // Map analog reading from 0 ~ 4095 to 32737 ~ 0 for use as an axis reading
    int offSet = 1000;
    int deadZone = 4000;
    int reductionFactor = 1200;
    int maxValue = 32737;

    // Map analog reading from 0 ~ 4095 to 32737 ~ 0 for use as an axis reading
    int adjustedValueX = map(potValueX, 0, 4095, maxValue, 0) - offSet;
    int moveX = (maxValue / 2 - adjustedValueX);
    if (abs(moveX) < deadZone) {
        moveX = 0;
    }
    moveX = moveX / reductionFactor;

    int adjustedValueY = map(potValueY, 0, 4095, maxValue, 0) - offSet;
    int moveY = (maxValue / 2 - adjustedValueY);
    if (abs(moveY) < deadZone) {
        moveY = 0;
    }
    moveY = moveY / reductionFactor;

    if (bleMouse.isConnected()) {
        //  Serial.println("Scroll Down");
        bleMouse.move(moveX, moveY, 0);

        if (touchRead(touchButtonPin) < 30) {
            bleMouse.press(MOUSE_LEFT);
        } else {
            bleMouse.release(MOUSE_LEFT);
        }

        if (analogRead(buttonMode) < 30) {
            modeJoystick++;
            if (modeJoystick == MODES.length() - 1) {
                modeJoystick = 0;
            } 
        }
    }
}
