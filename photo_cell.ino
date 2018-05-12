#include <CircularBuffer.h>
#include "RunningMedian.h"

#define SAMPLE_SIZE 3
#define ALERT_AFTER_MINUTES 10
#define SAFEGUARD_ALERT_AFTER_MINUTES 60
#define BUZZER_PIN 8
#define PHOTOCELL_PIN A0
#define MEDIAN_SAMPLING_FREQUENCY 200
#define SAMPLING_FREQUENCY 30
#define TONE_INTERVALL_MILLIS 300

unsigned long triggerAlertAt = -1;
unsigned long safeguardAlertAt = -1;
unsigned long lastSampleAt = -1;
unsigned long lastMedianSampleAt = -1;
unsigned long swapToneAt = -1;

bool inUse = false;
bool playAlert = false;
bool toneState = 0;

CircularBuffer<int,SAMPLE_SIZE> medianSamples;
RunningMedian samples = RunningMedian(10);

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  triggerAlertAt = millis() + milisecondsFor(ALERT_AFTER_MINUTES);
  safeguardAlertAt = millis() + milisecondsFor(SAFEGUARD_ALERT_AFTER_MINUTES);
  lastSampleAt = millis();
  lastMedianSampleAt = millis();
  swapToneAt = millis();
}

unsigned long milisecondsFor(long minutes){
  return minutes*60*1000;
}

void loop() { 
  addSample();
  
  processState( parseSamples() );

  checkAlerts();
  
  toneIt();
}

void addSample(){
  if(millis() > lastSampleAt){
    samples.add( analogRead(PHOTOCELL_PIN) );
    addMedianSample( samples.getMedian() );
    
    lastSampleAt = millis() + SAMPLING_FREQUENCY;
  }
}

void addMedianSample(long v){
  if(millis() > lastMedianSampleAt){
    medianSamples.push(v);
    lastMedianSampleAt = millis() + MEDIAN_SAMPLING_FREQUENCY;
  }
}

void processState(int state){
  switch(state){
    case -1:
      digitalWrite(LED_BUILTIN, HIGH);
      inUse = false;
      startCounterForAlert();
      break;
    case 1:
      digitalWrite(LED_BUILTIN, LOW);
      inUse = true;
      resetAlert();
      break;
    case 2:
      // state did not change
    break;
  }
}

void checkAlerts(){
  checkAlertTrigger();
  checkSafeGuardAlertTrigger();
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
  playAlert = true;
}

void stopBuzz(){
  playAlert = false;
  safeguardAlertAt = millis() + milisecondsFor(SAFEGUARD_ALERT_AFTER_MINUTES);
}

void toneIt(){
  if(playAlert == true){
    if(millis() > swapToneAt){
      toneState = !toneState;
      swapToneAt = millis() + TONE_INTERVALL_MILLIS;
      toneState ? tone(BUZZER_PIN,330) : noTone(BUZZER_PIN);
    }
  }else{
    noTone(BUZZER_PIN);
  }
}

int parseSamples(){
  int returnValue = 2;
  if(medianSamples.size() < SAMPLE_SIZE){ return returnValue; }

  if( abs(medianSamples.last() - medianSamples[SAMPLE_SIZE-2] ) < 20 
        && abs(medianSamples.first() - medianSamples.last()) > 20 ){
    if(medianSamples.first() < medianSamples.last()){
      returnValue = 1;
    }else{
      returnValue = -1;
    }
  }else{
    returnValue = 2;
  }  
  
  return returnValue;
}

