#include <CircularBuffer.h>

#define BUFFER_SIZE 15
#define PARSE_BLOCK 5
#define TREND_SIZE 3 // cannot change
#define ALERT_AFTER_MINUTES 1
#define SAFEGUARD_ALERT_AFTER_MINUTES 60
#define BUZZER 8
#define UPDATE_FREQUENCY 50

int photocellPin = 0;
unsigned long triggerAlertAt = -1;
unsigned long safeguardAlertAt = -1;
bool inUse = false;
int averageForBlocks[TREND_SIZE];
int start, end = 0;

CircularBuffer<int,BUFFER_SIZE> buffer;

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);  
  digitalWrite(LED_BUILTIN, HIGH);
  triggerAlertAt = millis() + milisecondsFor(ALERT_AFTER_MINUTES);
  safeguardAlertAt = millis() + milisecondsFor(SAFEGUARD_ALERT_AFTER_MINUTES);
}

unsigned long milisecondsFor(long minutes){
  return minutes*60*1000;
}

void loop() { 
  int v = analogRead(photocellPin); 
  buffer.push(v);
  Serial.println(v);
  int state = parseTrend();
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
    break;
  }
   checkAlertTrigger();
   checkSafeGuardAlertTrigger();
   delay(UPDATE_FREQUENCY); 
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

int parseTrend(){
  int returnValue = 2;
  if(buffer.size() == BUFFER_SIZE){
    for(int i=0; i<TREND_SIZE; i++){
      start = max(PARSE_BLOCK*i-1, 0); 
      end = PARSE_BLOCK*(i+1)-1;
      averageForBlocks[i] = averageFor(start, end);
    }
    // if the tail of the array has similar average AND there is a difference between the tail and the head
    if(abs(averageForBlocks[2]-averageForBlocks[1])<20 && abs(averageForBlocks[0] - averageForBlocks[2]) > 20){
      if(averageForBlocks[0] < averageForBlocks[1]){
        returnValue = 1;
      }else{
        returnValue = -1;
      }
    }else{
      returnValue = 2;
    }
  }else{
    Serial.println("Not enough values");
  }
  return returnValue;
}

int averageFor( int start, int end){
  int tmp = 0;
  for(int i = start; i<=end; i++){ tmp += buffer[i]; }
  return tmp/PARSE_BLOCK;
}

