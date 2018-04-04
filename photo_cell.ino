#include <CircularBuffer.h>
#include "RunningMedian.h"

#define SAMPLE_SIZE 3
#define ALERT_AFTER_MINUTES 1
#define SAFEGUARD_ALERT_AFTER_MINUTES 60
#define BUZZER 8
#define UPDATE_FREQUENCY 20
#define SAMPLING_FREQUENCY 200

int photocellPin = A0;
unsigned long triggerAlertAt = -1;
unsigned long safeguardAlertAt = -1;
bool inUse = false;

CircularBuffer<int,SAMPLE_SIZE> samples;
RunningMedian samplesMedian = RunningMedian(10);
unsigned long lastSampleAt = -1;

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  triggerAlertAt = millis() + milisecondsFor(ALERT_AFTER_MINUTES);
  safeguardAlertAt = millis() + milisecondsFor(SAFEGUARD_ALERT_AFTER_MINUTES);
  lastSampleAt = millis();
}

unsigned long milisecondsFor(long minutes){
  return minutes*60*1000;
}

void loop() { 
  int v = analogRead(photocellPin); 
  samplesMedian.add(v);  
  int state = parseSamples();

  switch(state){
    case -1:
      Serial.println("LED_BUILTIN, HIGH");
      digitalWrite(13, HIGH);
      inUse = false;
      startCounterForAlert();
      break;
    case 1:
    Serial.println("LED_BUILTIN, LOW");
      digitalWrite(13, LOW);
      inUse = true;
      resetAlert();
      break;
    case 2:
    break;
  }
   checkAlertTrigger();
   checkSafeGuardAlertTrigger();
   delay(UPDATE_FREQUENCY);
   addSample(samplesMedian.getMedian());
}

void addSample(long v){
  if(millis() > lastSampleAt){
    samples.push(v);
    lastSampleAt = millis() + SAMPLING_FREQUENCY;
  }
}

void startCounterForAlert(){
  triggerAlertAt = millis() + milisecondsFor(ALERT_AFTER_MINUTES);
}

void resetAlert(){
  stopBuzz();
}

void checkSafeGuardAlertTrigger(){
  if(millis() > safeguardAlertAt){ buzzThis(); }
}

void checkAlertTrigger(){
  if(!inUse){
    if(millis() > triggerAlertAt){ buzzThis(); }
  }
}

void buzzThis(){
  tone(BUZZER, 1000);  
}

void stopBuzz(){
  noTone(BUZZER);
  safeguardAlertAt = millis() + milisecondsFor(SAFEGUARD_ALERT_AFTER_MINUTES);
}

int parseSamples(){
  int returnValue = 2;
  if(samples.size() == SAMPLE_SIZE){
    if(abs(samples[SAMPLE_SIZE-1]-samples[SAMPLE_SIZE-2])<20 && abs(samples[0] - samples[SAMPLE_SIZE-1]) > 20){
      if(samples[0] < samples[SAMPLE_SIZE-1]){
        returnValue = 1;
      }else{
        returnValue = -1;
      }
    }else{
      returnValue = 2;
    }  
  }
  return returnValue;
}

