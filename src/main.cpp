
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <BleMouse.h>
#include <BleConnectionStatus.h>

const int potPinX = 34;                // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int potPinY = 35;                // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int numberOfPotSamples = 5;     // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 5;    // Delay in milliseconds between pot samples
const int delayBetweenHIDReports = 100; // Additional delay in milliseconds between HID reports

BleMouse bleMouse("La manette qui sent drôle", "Emile Maher", 69);

void setup()
{
    Serial.begin(9600);
/*
    // Create the BLE Device
    BLEDevice::init("Joy-Livier");
    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);
    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);
    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pCharacteristic->addDescriptor(new BLE2902());
    // Start the service
    pService->start();
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    //BLEDevice::startAdvertising();
    Serial.println("Waiting for a client connection to notify...");
*/
    bleMouse.begin();
}

void loop()
{
    /*
    // notify changed value
    if (deviceConnected) {
        pCharacteristic->setValue((uint8_t *)&value, 4);
        pCharacteristic->notify();
        value++;
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }
*/
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
    potValueX = potValueX / numberOfPotSamples;
    potValueY = potValueY / numberOfPotSamples;

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
      //  Serial.print("moveX: ");
     //   Serial.print(moveX);
    }
/*
    delay(delayBetweenHIDReports);

    // The code below (apart from the 2 closing braces) is for pot value degugging, and can be removed
    // Print readings to serial port
    Serial.print("Sent: ");
    Serial.print(adjustedValueX);
    Serial.print("\tRaw Avg: ");
    Serial.print(potValue);
    Serial.print("\tRaw: {");

    // Iterate through raw pot values, printing them to the serial port
    for (int i = 0; i < numberOfPotSamples; i++)
    {
        Serial.print(potValues[i]);

        // Format the values into a comma seperated list
        if (i == numberOfPotSamples - 1)
        {
            Serial.println("}");
        }
        else
        {
            Serial.print(", ");
        }
    }
    */
}
