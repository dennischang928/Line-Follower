#include "printf.h"
#include "RF24.h"

#define RPWM_PIN A3
#define RENABLE_PIN 2
#define RBRAKE_PIN 3
#define RDIR_PIN 4

#define LPWM_PIN A2
#define LENABLE_PIN 5
#define LBRAKE_PIN 6
#define LDIR_PIN 7

#define CE_PIN A1
#define CSN_PIN 10

#include <Tone.h>

Tone RMotor;
Tone LMotor;

RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = {"1Node", "2Node"}; // Computer as the master

float Speed = 100.2;

class Motor
{
public:
    unsigned long timer2 = 0;
    int TargetVelocity = 0;
    int CurrentVelocity = 0;
    float slope = 0;
    int time_interval = 10;

    byte EN_PIN = 0;
    byte DIR_PIN = 0;
    byte BRAKE_PIN = 0;
    byte PWM_PIN = 0;

    void SetupMotor(byte DIR_PIN_, byte BRAKE_PIN_, byte PWM_PIN_, byte EN_PIN_, Tone* MotorFrequencyTuning)
    {
        DIR_PIN = DIR_PIN_;
        BRAKE_PIN = BRAKE_PIN_;
        PWM_PIN = PWM_PIN_;
        EN_PIN = EN_PIN_;
    }
    void setTargetVelocity(int TargetVelocity_)
    {
        TargetVelocity = TargetVelocity_;
        slope = (TargetVelocity - CurrentVelocity) / time_interval;
        timer2 = millis();
    }

    boolean Motor_Loop()
    {
        CurrentVelocity = CurrentVelocity + (millis() - timer2) * slope;
        TuneSpeed(CurrentVelocity, EN_PIN, BRAKE_PIN, DIR_PIN, MotorFrequencyTuning);
    }
};

Motor MotorA;

void setup()
{
    MotorA.SetupMotor(1, 2, 3);

    Serial.begin(115200);
    pinMode(RPWM_PIN, OUTPUT);
    pinMode(RENABLE_PIN, OUTPUT);
    pinMode(RBRAKE_PIN, OUTPUT);
    pinMode(RDIR_PIN, OUTPUT);

    pinMode(LPWM_PIN, OUTPUT);
    pinMode(LENABLE_PIN, OUTPUT);
    pinMode(LBRAKE_PIN, OUTPUT);
    pinMode(LDIR_PIN, OUTPUT);

    pinMode(A6, INPUT);
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

    radio.openWritingPipe(address[1]);
    radio.openReadingPipe(1, address[0]);

    radio.startListening(); // put radio in RX mode

    digitalWrite(RENABLE_PIN, HIGH);
    digitalWrite(RBRAKE_PIN, HIGH);
    digitalWrite(RDIR_PIN, LOW);

    digitalWrite(LENABLE_PIN, HIGH);
    digitalWrite(LBRAKE_PIN, HIGH);
    digitalWrite(LDIR_PIN, HIGH);

    RMotor.begin(RPWM_PIN);
    LMotor.begin(LPWM_PIN);
    RMotor.play(20000);
    LMotor.play(20000);

    RMotor.play(0);
    LMotor.play(0);
}

long timer = millis();
char MessageReceived[2] = "";

void loop()
{
    MotorA.setTargetVelocity();
}

// void loop()
// {
//     if (radio.available())
//     {
//         radio.read(&MessageReceived, sizeof(String)); // fetch payload from FIFO
//         Serial.println(MessageReceived);
//         timer = millis();
//     }
//     if (millis() - timer < 200)
//     {
//         if (MessageReceived[1] == 'F')
//             MotorDrive(5, 5);
//         else if (MessageReceived[1] == 'R')
//             MotorDrive(5, -5);
//         else if (MessageReceived[1] == 'B')
//             MotorDrive(-5, -5);
//         else if (MessageReceived[1] == 'L')
//             MotorDrive(-5, 5);
//     }
//     else if (millis() - timer < 400)
//     {
//         MotorDrive(1, 1);
//     }
// }