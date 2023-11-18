#include <stdio.h>
#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = {"1Node", "2Node"}; // Computer as the master

float Speed = 104; // 0 - 1024

void setup()
{
    Serial.begin(115200);
    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
            Serial.println(F("radio hardware is not responding!!"));
        } // hold in infinite loop
    }

    radio.setPALevel(RF24_PA_MAX); // RF24_PA_MAX is default.

    // save on transmission time by setting the radio to only transmit the
    // number of bytes we need to transmit a float

    radio.openWritingPipe(address[0]);

    radio.openReadingPipe(1, address[1]);
    radio.openWritingPipe(address[0]);

    // put radio in TX mode
}

void loop()
{
    if (Serial.available() > 0)
    {
        radio.stopListening();
        char Message[20] = "";
        String Received = Serial.readStringUntil('\n');
        strcpy(Message, Received.c_str());
        bool report = radio.write(&Message, sizeof(Message));
        Serial.println(report ? "Sent" : "Failed to Send");
    }
}
