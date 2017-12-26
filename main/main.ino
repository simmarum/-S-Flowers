#include <TimerOne.h>

#define ledPin 2
#define second 1000000
void CounterTime(void);
void blinkLed(void);
long timeToBlink = 5;
long countToBlink = 0;
bool whetherBlinkLed = false;

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);
  //Ustawienie przerwania co 1s
  Timer1.initialize(second);
  //Przyczepienie funkcji mruganie do timera1
  Timer1.attachInterrupt(CounterTime);
}
 
void loop(){
  if(whetherBlinkLed){
    blinkLed();
    whetherBlinkLed = !whetherBlinkLed;
  }
}

void CounterTime(){
  countToBlink++;
  if( countToBlink == timeToBlink){
    whetherBlinkLed = !whetherBlinkLed;
    countToBlink = 0;
  }
}

void blinkLed(){
  Serial.println(!digitalRead(ledPin));
  digitalWrite(ledPin, !digitalRead(ledPin));
}
