#include "RF24.h"
#include <EasyTransferI2C.h>
#include <Wire.h>
#include <Tone.h>

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

EasyTransferI2C ET;
EasyTransferI2C ER;

struct ET_DATA_STRUCTURE
{
  char cmd = '_';
  byte order = 0;
};

struct ER_DATA_STRUCTURE
{
  float result;
};

ER_DATA_STRUCTURE receiveData;
ET_DATA_STRUCTURE sendData;

#define I2C_MASTER_ADDRESS 1
#define I2C_SLAVE_ADDRESS 9

RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = {"1Node", "2Node"}; // Computer as the master

boolean IsDebugMode = false;
boolean IsPIDLoopStart = false;
float Kp = 500;
float Kd = 0;
int Kdd = 10;
float MaxSpeed = 2000;

class Motor
{
public:
  Tone MotorFrequencyHelper;
  unsigned long timer2 = 0;
  int TargetVelocity = 0;
  int CurrentVelocity = 0;
  float acceleration = 0;
  float time_interval = 6.;
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
    MotorFrequencyHelper.play(1000);
    delay(2);
    TuneSpeed(0);
  }

  void setTargetVelocity(int TargetVelocity_)
  {
    TargetVelocity = TargetVelocity_;
    acceleration = (TargetVelocity - CurrentVelocity) / time_interval;
    time_used = 0;
    timer2 = millis();
  }

  void TuneSpeed(int Value)
  {
    digitalWrite(DIR_PIN, Value < 0 ? LOW : HIGH);
    if (Value != 0)
    {
      digitalWrite(BRAKE_PIN, HIGH);
      int mappedValue = map(constrain(abs(Value), 0, 16384), 1, 16384, 1000, 26000);
      MotorFrequencyHelper.play(mappedValue);
    }
    else
    {
      digitalWrite(BRAKE_PIN, LOW);
    }
  }

  boolean Motor_Loop()
  {
    unsigned long currentMillis = millis();
    unsigned long timeDiff = currentMillis - timer2;
    time_used += timeDiff;

    if (time_used >= time_interval)
    {
      CurrentVelocity = TargetVelocity;
    }
    else
    {
      CurrentVelocity += timeDiff * acceleration;
    }

    timer2 = currentMillis;
    TuneSpeed(CurrentVelocity);
  }
};

Motor MotorRight;
Motor MotorLeft;

void FollowerDrive(int L_Velocity, int R_Velocity)
{
  MotorRight.TuneSpeed(R_Velocity);
  MotorLeft.TuneSpeed(-L_Velocity);
}

void wireSetup()
{
  Wire.begin(I2C_MASTER_ADDRESS);
  // start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  ET.begin(details(sendData), &Wire);
  ER.begin(details(receiveData), &Wire);
  // define handler function on receiving data
  Wire.onReceive(receive);
}

void receive(int numBytes) {}

void setup()
{
  Serial.begin(115200);
  wireSetup();
  radio.begin();

  radio.setPALevel(RF24_PA_HIGH); // RF24_PA_MAX is default.
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);
  radio.startListening(); // put radio in RX mode

  MotorRight.SetupMotor(RDIR_PIN, RBRAKE_PIN, RPWM_PIN, RENABLE_PIN); // byte DIR_PIN_, byte BRAKE_PIN_, byte PWM_PIN_, byte EN_PIN_
  MotorLeft.SetupMotor(LDIR_PIN, LBRAKE_PIN, LPWM_PIN, LENABLE_PIN);
  FollowerDrive(0, 0);
}

long timer = millis();
char MessageReceived[30] = "";
boolean IsCalibrating = false;
long ErrorSendTimer = millis();
float Error = 0;
float LastError = 0;

struct TXMESSAGESTRUCT
{
  char Command;
  float Suffix;
};

TXMESSAGESTRUCT TxMessage;
float thiskd = 0;
float lastkddv = 0;

void PIDLoop()
{

  if (IsPIDLoopStart)
  {
    GetError();
    int output = Kp * Error + Kd * (Error - LastError);
    output = constrain(output, -2 * MaxSpeed, 2 * MaxSpeed);
    if (Error > 0)
      FollowerDrive(MaxSpeed, MaxSpeed - output);
    else
      FollowerDrive(MaxSpeed - abs(output), MaxSpeed);

    LastError = Error;
  }
}

void loop()
{
  DebugLoop();
  PIDLoop();
  if (IsCalibrating == true)
  {
    CalibrationCommandHandler();
  }

  CommanderLoop();
}

void DebugLoop()
{
  if (millis() - ErrorSendTimer >= 1000 && IsDebugMode == true)
  {
    radio.stopListening(); // put radio in TX mode
    GetError();
    TxMessage.Command = 'E';
    TxMessage.Suffix = Error;
    ErrorSendTimer = millis();
    delay(1);
    radio.write(&TxMessage, sizeof(TxMessage));
    radio.startListening(); // put radio in TX mode
  }
}

void GetError()
{
  sendData.cmd = 'E';
  sendData.order = 0;
  ET.sendData(I2C_SLAVE_ADDRESS);
  if (ER.receiveData())
  {
    Error = receiveData.result;
  }
  Serial.println(Error);
}

void CommanderLoop()
{
  if (radio.available())
  {
    radio.read(&MessageReceived, sizeof(MessageReceived)); // fetch payload from FIFO
    Serial.println(MessageReceived);

    if (MessageReceived[0] == 'D')
    {
      DrivingCommand(MessageReceived[1]);
    }
    else if (MessageReceived[0] == 'T')
    {
      TuningCommandHandler(MessageReceived);
    }
    else if (MessageReceived[0] == 'B')
    {
      IsDebugMode = MessageReceived[1] == 'E' ? true : false;
      Serial.println(IsDebugMode);
    }
    else if (MessageReceived[0] == 'C')
    {
      while (ER.receiveData())
      {
        Serial.println(receiveData.result);
      };

      sendData.cmd = 'C';
      sendData.order = 0;
      for (int i = 0; i < 10; i++)
      {
        ET.sendData(I2C_SLAVE_ADDRESS);
        delay(10);
        if (ER.receiveData())
        {
          if (receiveData.result == -1.0)
          {
            IsCalibrating = true;
            Serial.println("Calibration Start");
            break;
          }
          else
          {
            Serial.println("Calibration Failed");
          }
        }
        else
        {
          Serial.println("Calibration Failed");
        }
      }
    }
    else if (MessageReceived[0] == 'K')
    {
      IsPIDLoopStart = MessageReceived[1] == 'E' ? true : false;
      if (IsPIDLoopStart == false)
      {
        FollowerDrive(0, 0);
      }
    }
  }
}

void CalibrationCommandHandler()
{
  sendData.cmd = 'c';
  ET.sendData(I2C_SLAVE_ADDRESS);
  if (ER.receiveData())
  {
    if (receiveData.result == -2.0)
    {
      FollowerDrive(500, -500);
      Serial.println("Not Done Calibration");
    }
    else if (receiveData.result == -3.0)
    {
      IsCalibrating = false;
      FollowerDrive(0, 0);
      Serial.println("Calibration Done");
    }
  }
}

void TuningCommandHandler(String inputCommand)
{
  int pStartIndex = inputCommand.indexOf('P');
  int pEndIndex = inputCommand.indexOf('D');
  String pValueString = inputCommand.substring(pStartIndex + 1, pEndIndex);
  Kp = pValueString.toFloat();

  int dStartIndex = inputCommand.indexOf('D');
  int dEndIndex = inputCommand.indexOf('M');
  String dValueString = inputCommand.substring(dStartIndex + 1, dEndIndex);
  Kd = dValueString.toFloat();

  int maxspeedStartIndex = inputCommand.indexOf("MxSP");
  String maxspeedValueString = inputCommand.substring(maxspeedStartIndex + 4);
  MaxSpeed = maxspeedValueString.toInt();

  Serial.print(Kp);
  Serial.print("  ");
  Serial.print(Kd);
  Serial.print("  ");
  Serial.println(MaxSpeed);
}

void DrivingCommand(char Direction)
{
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