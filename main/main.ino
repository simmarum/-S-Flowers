#include <DallasTemperature.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <TimerOne.h>

#define second 1000000

void CounterTime(void);

/// General counter time to refresh data from sensors
long iterator = 0;
const long samples = 18;
long timeToGetAllData = 36; /// time to get all date (18 times)
long countToGetAllData = 0;
long timeToGetData = long(timeToGetAllData/int(samples)); /// time between every sample
long countToGetData = 0;
bool whetherGetData = false;


/// Soil humidity
float SoilDataFirst[samples];
float SoilDataSecond[samples];
float getSoilHumidity(int input);
float getMeanHumidity(int input);

/// DS18B20 - temperature (small black)
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS); // this sensor use ONEWIRE connections
DallasTemperature tempSensors(&oneWire); // to make it easier
float TempBlackData[samples];
float getTempBlack(void);
float getMeanTempBlack(void);

/// DEBUG
void printTableFloat(float input[]){
  for(int i=0;i<samples;i++){
    Serial.print(String(input[i]) + ", ");
  }
  Serial.println();
}
void setup()
{
  
  Serial.begin(115200);
  Timer1.initialize(second); //Set interrupt every 1 second
  Timer1.attachInterrupt(CounterTime);
  
  tempSensors.begin(); //Start the DallasTemperature Library
}
 
void loop(){
  if(whetherGetData){
    if(iterator == samples){
      Serial.println("TODO - wyslac dane przez wifi");
      float meanFirst = getMeanHumidity(A0);
      Serial.print(meanFirst,2);
      Serial.print("\t");
      float meanSecond = getMeanHumidity(A1);
      Serial.print(meanSecond,2);
      Serial.print("\t");
      float meanTempBlack = getMeanTempBlack();
      Serial.print(meanTempBlack,2);
      Serial.print("\t");
      Serial.println();
      iterator = 0;
      countToGetData = 0;
      whetherGetData = false;
    } else {
      Serial.println(iterator);
      SoilDataFirst[iterator] = getSoilHumidity(A0);
      SoilDataSecond[iterator] = getSoilHumidity(A1);
      TempBlackData[iterator] = getTempBlack();
      iterator++;
      whetherGetData = false;
    }
  }
}

void CounterTime(){
  countToGetData++;
  if( countToGetData == timeToGetData){
    whetherGetData = true;
    countToGetData = 0;
  }
}

float getSoilHumidity(int input){
  long wartosc_A = analogRead(input);
  float value = float(wartosc_A);
  value *= 100;
  value /=1023;
  return value; // return in percent
}

float getMeanHumidity(int input){
  /// find max and min value in array
  float minV = 1024.0; // max value from analog read is 1023 so 1024 is enough
  float maxV = 0; // no comment ;p
  float sum = 0.0; // sum of all data
  for(int i=0;i<samples;i++){
    float value = 0.0;
    if (input == A0){
      value = SoilDataFirst[i];
    } else if (input == A1){
      value = SoilDataSecond[i];
    }
    minV = min(minV,value);
    maxV = max(maxV,value);
    sum = sum + value;
  }
  /// addition all values and substraction min and max value then divide by samples-2
  sum = sum - (minV + maxV);
  sum = sum / (samples-2);
  return sum;
}

float getTempBlack(){
  tempSensors.requestTemperatures(); // Tell the DS18B20 to get make a measurement
  float value = tempSensors.getTempCByIndex(0); // Get that temperature
  return value;
}

float getMeanTempBlack(){
  /// find max and min value in array
  float minV = 1024.0; // max temperature
  float maxV = -1024.0; // min temperature
  float sum = 0.0; // sum of all data
  for(int i=0;i<samples;i++){
    float value = TempBlackData[i];
    minV = min(minV,value);
    maxV = max(maxV,value);
    sum = sum + value;
  }
  /// addition all values and substraction min and max value then divide by samples-2
  sum = sum - (minV + maxV);
  sum = sum / (samples-2);
  return sum;
}

