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

RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = {"1Node", "2Node"}; // Computer as the master

class Motor
{
public:
    Tone MotorFrequencyHelper;
    unsigned long timer2 = 0;
    int TargetVelocity = 0;
    int CurrentVelocity = 0;
    float acceleration = 0;
    float time_interval = 10.;
    long time_used = 0;

    byte EN_PIN = 0;
    byte DIR_PIN = 0;
    byte BRAKE_PIN = 0;
    byte PWM_PIN = 0;

    void SetupMotor(byte DIR_PIN_, byte BRAKE_PIN_, byte PWM_PIN_, byte EN_PIN_)
    {
        DIR_PIN = DIR_PIN_;
        BRAKE_PIN = BRAKE_PIN_;
        PWM_PIN = PWM_PIN_;
        EN_PIN = EN_PIN_;
        pinMode(PWM_PIN, OUTPUT);
        pinMode(EN_PIN, OUTPUT);
        pinMode(BRAKE_PIN, OUTPUT);
        pinMode(DIR_PIN, OUTPUT);
        digitalWrite(EN_PIN, HIGH);
        digitalWrite(BRAKE_PIN, HIGH);
        digitalWrite(DIR_PIN, LOW);

        MotorFrequencyHelper.begin(PWM_PIN);
        MotorFrequencyHelper.play(23000);
        delay(2);
        MotorFrequencyHelper.play(0);
    }

    void setTargetVelocity(int TargetVelocity_) //
    {
        TargetVelocity = TargetVelocity_;
        acceleration = (TargetVelocity - CurrentVelocity) / time_interval;
        time_used = 0;
        timer2 = millis();
    }

    void TuneSpeed(int Value)
    {
        digitalWrite(EN_PIN, HIGH);

        digitalWrite(DIR_PIN, Value < 0 ? LOW : HIGH);

        if (Value != 0)
        {
            digitalWrite(BRAKE_PIN, HIGH);
            MotorFrequencyHelper.play(map(constrain(abs(Value), 0, 16384), 1, 16384, 2500, 26000));
        }
        else
        {
            digitalWrite(BRAKE_PIN, LOW);
        }
    }
    boolean Motor_Loop()
    {
        time_used = time_used + (millis() - timer2); // accumulate the time used to accelerate
        if (time_used >= time_interval)
        { // if time used is larger than our expected time interval => directly set the motor to our target Velocity
            CurrentVelocity = TargetVelocity;
        }
        else
        {
            CurrentVelocity = CurrentVelocity + (millis() - timer2) * acceleration; // v = u + at;
            timer2 = millis();
        }
        TuneSpeed(CurrentVelocity);
        // Serial.println(adf);
        // TuneSpeed(CurrentVelocity, EN_PIN, BRAKE_PIN, DIR_PIN, MotorFrequencyTuning);
    }
};

Motor MotorRight;
Motor MotorLeft;

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
    radio.openWritingPipe(address[1]);
    radio.openReadingPipe(1, address[0]);
    radio.startListening(); // put radio in RX mode

    // RMotor.begin(RPWM_PIN);
    // LMotor.begin(LPWM_PIN);
    // RMotor.play(20000);
    // LMotor.play(20000);

    // RMotor.play(0);
    // LMotor.play(0);

    MotorRight.SetupMotor(RDIR_PIN, RBRAKE_PIN, RPWM_PIN, RENABLE_PIN); // byte DIR_PIN_, byte BRAKE_PIN_, byte PWM_PIN_, byte EN_PIN_
    MotorLeft.SetupMotor(LDIR_PIN, LBRAKE_PIN, LPWM_PIN, LENABLE_PIN);
    MotorRight.setTargetVelocity(0);
    MotorLeft.setTargetVelocity(0);
}

long timer = millis();
char MessageReceived[2] = "";

// void loop()
// {
//     MotorRight.Motor_Loop();

//     while (1)
//     {
//         MotorRight.Motor_Loop();
//     }
// }

void loop()
{
    if (radio.available())
    {
        radio.read(&MessageReceived, sizeof(String)); // fetch payload from FIFO
        Serial.println(MessageReceived);
        if (MessageReceived[1] == 'F')
        {

            FollowerDrive(3000, 3000);
        }
        else if (MessageReceived[1] == 'R')
        {

            FollowerDrive(2000, -2000);
        }
        else if (MessageReceived[1] == 'B')
        {

            FollowerDrive(-3000, -3000);
        }
        else if (MessageReceived[1] == 'L')
        {

            FollowerDrive(-2000, 2000);
        }
        else if (MessageReceived[1] == 'S') // stop
        {
            FollowerDrive(0, 0);
        }
    }

    MotorRight.Motor_Loop();
    MotorLeft.Motor_Loop();
}

void FollowerDrive(int L_Velocity, int R_Velocity)
{
    MotorRight.setTargetVelocity(R_Velocity);
    MotorLeft.setTargetVelocity(-L_Velocity);
}