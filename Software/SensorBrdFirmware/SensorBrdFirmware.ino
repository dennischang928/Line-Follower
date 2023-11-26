#include <Wire.h>
#include <EasyTransferI2C.h>
#include <EEPROM.h>

// create object
EasyTransferI2C ET;
EasyTransferI2C ER;

struct ET_DATA_STRUCTURE
{
  float result;
};
struct ER_DATA_STRUCTURE
{
  char cmd = "";
  byte order = 0;
};
ER_DATA_STRUCTURE receiveData;
ET_DATA_STRUCTURE sendData;
// define slave i2c address
#define I2C_MASTER_ADDRESS 1
#define I2C_SLAVE_ADDRESS 9

int selectPin[] = {2, 3, 4, 5, 6, 7};
float sensorValue[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float sensorMin[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
float sensorMax[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int resultant[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float Drift = 0, lastDrift = 0;
float error = 0;
unsigned long timer;
boolean trigger = false;
boolean IsCalibrationStart = 0;

void setup()
{

  wireSetup();

  Serial.begin(115200);
  for (int i = 0; i <= 5; i++)
    pinMode(selectPin[i], OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  trigger = false;

  for (int i = 0; i < 15; i++)
  {
    EEPROM.get(i * 4, sensorMin[i]);
    // Serial.print(sensorMin[i]);
    // Serial.print("  ");
    // Serial.print(sensorMin[i]);
  }
  Serial.println("");
  for (int i = 0; i < 15; i++)
  {
    EEPROM.get(15 * 4 + i * 4, sensorMax[i]);
    // Serial.print(sensorMax[i]);
    // Serial.print("  ");
    // Serial.print(sensorMax[i]);
  }
  // Serial.println("");
}

void loop()
{

  float avg = 0, sum = 0;
  boolean online = false;
  for (int i = 0; i <= 14; i++)
  {
    sensorValue[i] = (float)readSensor(14 - i);
    resultant[i] = map(sensorValue[i], sensorMin[i], sensorMax[i], 0, 3);

    if (resultant[i] > 1)
      online = true;

    avg += (float)(resultant[i]) * i;
    sum += (float)(resultant[i]);
  }

  if (!online)
    Drift = lastDrift;
  else
  {
    lastDrift = avg / sum;
    Drift = lastDrift;
  }

  error = Drift - 7.0;
  checkWire();
  cali();
}

int readSensor(int ID)
{
  int ans;
  if (ID <= 7)
  {
    digitalWrite(selectPin[0], ID & 1);
    digitalWrite(selectPin[1], (ID & 2) >> 1);
    digitalWrite(selectPin[2], (ID & 4) >> 2);
    delay(1);
    ans = analogRead(A0);
    delay(1);
  }
  else
  {
    digitalWrite(selectPin[3], (ID - 8) & 1);
    digitalWrite(selectPin[4], ((ID - 8) & 2) >> 1);
    digitalWrite(selectPin[5], ((ID - 8) & 4) >> 2);
    delay(1);
    ans = analogRead(A1);
    delay(1);
  }
  return ans;
}

void wireSetup()
{
  Wire.begin(I2C_SLAVE_ADDRESS);
  // start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  ET.begin(details(sendData), &Wire);
  ER.begin(details(receiveData), &Wire);
  // define handler function on receiving data
  Wire.onReceive(receive);
}

void receive(int numBytes) {}

void checkWire()
{
  if (ER.receiveData())
  {
    switch (receiveData.cmd)
    {
    case 'C':
      sendData.result = -1;
      ET.sendData(I2C_MASTER_ADDRESS);
      trigger = true;
      IsCalibrationStart = 1;
      timer = millis();
      for (int i = 0; i < 15; i++)
      {
        sensorMax[i] = 0;
        sensorMin[i] = 1023;
      }

      break;
    case 'c':
      sendData.result = trigger ? -2.0 : -3.0;
      ET.sendData(I2C_MASTER_ADDRESS);

      break;

      // case 'S':
      //   sendData.result = sensorValue[receiveData.order];
      //   sendData.result = temp;
      //   ET.sendData(I2C_MASTER_ADDRESS);
      //   break;

    case 'E':
      sendData.result = error;
      ET.sendData(I2C_MASTER_ADDRESS);
      break;
    }
  }
}

void cali()
{
  if (IsCalibrationStart == 1)
  {

    if (millis() - timer <= 5000 && trigger == true)
    {
      for (int i = 0; i < 15; i++)
      {

        Serial.print(sensorMax[i]);
        Serial.print("   ");
        Serial.print(sensorMin[i]);
        Serial.print("   ");
      }
      for (int i = 0; i < 15; i++)
      {

        sensorValue[i] = readSensor(14 - i);

        if (sensorValue[i] > sensorMax[i])
          sensorMax[i] = sensorValue[i];
        if (sensorValue[i] < sensorMin[i])
          sensorMin[i] = sensorValue[i];
      }
      Serial.println("");
    }
    else
    {

      Serial.println("Calibration Done");
      for (int i = 0; i < 15; i++)
      {

        Serial.print(sensorMax[i]);
        Serial.print("   ");
        Serial.print(sensorMin[i]);
        Serial.print("   ");
      }
      Serial.println("");
      IsCalibrationStart = 0;
      trigger = false;

      for (int i = 0; i < 15; i++)
        EEPROM.put(i * 4, sensorMin[i]);
      for (int i = 0; i < 15; i++)
        EEPROM.put(15 * 4 + i * 4, sensorMax[i]);
    }
  }
}
