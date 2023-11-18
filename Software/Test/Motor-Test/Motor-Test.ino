
#define APWM_PIN A3
#define AENABLE_PIN 2
#define ABRAKE_PIN 3
#define ADIR_PIN 4

#define BPWM_PIN A2
#define BENABLE_PIN 5
#define BBRAKE_PIN 6
#define BDIR_PIN 7


#include <Tone.h>

Tone AMotor;
Tone BMotor;

void setup(){
  Serial.begin(115200);
  pinMode(APWM_PIN,OUTPUT);
  pinMode(AENABLE_PIN,OUTPUT);
  pinMode(ABRAKE_PIN,OUTPUT);
  pinMode(ADIR_PIN,OUTPUT);

  pinMode(BPWM_PIN,OUTPUT);
  pinMode(BENABLE_PIN,OUTPUT);
  pinMode(BBRAKE_PIN,OUTPUT);
  pinMode(BDIR_PIN,OUTPUT);

  digitalWrite(AENABLE_PIN, HIGH);
  digitalWrite(ABRAKE_PIN, HIGH);
  digitalWrite(ADIR_PIN, HIGH);


  digitalWrite(BENABLE_PIN, HIGH);
  digitalWrite(BBRAKE_PIN, HIGH);
  digitalWrite(BDIR_PIN, LOW);


  AMotor.begin(APWM_PIN);
  BMotor.begin(BPWM_PIN);
  AMotor.play(23000);
  BMotor.play(23000);
}

void loop(){
  AMotor.play(10000);
  BMotor.play(5000);
  delay(1000);
}