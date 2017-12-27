#include <dht11.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <TimerOne.h>
#include "DHT.h"
#include "WiFiEsp.h"

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(10, 11); // RX, TX
#endif

char ssid[] = "IronMaiden";         // your network SSID (name)
char pass[] = "72e3d0d9bfa2";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

#define second 1000000

void CounterTime(void);

/// General counter time to refresh data from sensors
long iterator = 0;
const long samples = 18;
long timeToGetAllData = 36; /// time to get all date (18 times)
long countToGetAllData = 0;
long timeToGetData = long(timeToGetAllData / int(samples)); /// time between every sample
long countToGetData = 0;
bool whetherGetData = false;


/// Soil humidity
#define SHFirst A0
#define SHSecond A1
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

/// Liquid level sensor
#define LLPin A2
float LiquidLevelData[samples];
float getLiquidLevel(int input);
float getMeanLiquidLevel(void);

/// DHT11 (Temperature and humidity (blue))
#define DHTPIN 4     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float TempBlueData[samples];
float HumBlueData[samples];
float getTempBlue();
float getHumBlue();
float getMeanTempBlue(void);
float getMeanHumBlue(void);

/// LED
#define ledPin 7

/// DEBUG
void printTableFloat(float input[]) {
  for (int i = 0; i < samples; i++) {
    Serial.print(String(input[i]) + ", ");
  }
  Serial.println();
}
void setup()
{
  digitalWrite(ledPin, HIGH);
  Serial.begin(115200);
  Timer1.initialize(second); //Set interrupt every 1 second
  Timer1.attachInterrupt(CounterTime);
  Serial1.begin(9600);    // initialize serial for ESP module
  WiFi.init(&Serial1);    // initialize ESP module
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true) {
      digitalWrite(ledPin, LOW);
      delay(500);
      digitalWrite(ledPin, HIGH);
      delay(500);
    }
  }
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    digitalWrite(ledPin, LOW);
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  digitalWrite(ledPin, HIGH);
  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  if (WiFi.ping("www.google.com")) {
    Serial.println("PING to google.com was succesfull");
  } else {
    Serial.println("PING to google.com was fail!!!");
    while (true) {
      digitalWrite(ledPin, LOW);
      delay(100);
      digitalWrite(ledPin, HIGH);
      delay(100);
    }
  }

  tempSensors.begin(); //Start the DallasTemperature Library
  dht.begin();
}

void loop() {
  if (whetherGetData) {
    if (iterator == samples) {
      Serial.println("\nTODO - wyslac dane przez wifi");
      float meanFirst = getMeanHumidity(SHFirst);
      Serial.print("\tSH1:");
      Serial.print(meanFirst, 2);
      float meanSecond = getMeanHumidity(SHSecond);
      Serial.print("\tSH2:");
      Serial.print(meanSecond, 2);
      float meanTempBlack = getMeanTempBlack();
      Serial.print("\tTBk:");
      Serial.print(meanTempBlack, 2);
      float meanLiquidLevel = getMeanLiquidLevel();
      Serial.print("\tLL:");
      Serial.print(meanLiquidLevel, 2);
      float meanTempBlue = getMeanTempBlue();
      Serial.print("\tTBe:");
      Serial.print(meanTempBlue, 2);
      float meanHumBlue = getMeanHumBlue();
      Serial.print("\tHBe:");
      Serial.print(meanHumBlue, 2);
      Serial.println();
      iterator = 0;
      countToGetData = 0;
      whetherGetData = false;
    } else {
      Serial.print(iterator+" ");
      SoilDataFirst[iterator] = getSoilHumidity(SHFirst);
      SoilDataSecond[iterator] = getSoilHumidity(SHSecond);
      TempBlackData[iterator] = getTempBlack();
      LiquidLevelData[iterator] = getLiquidLevel(LLPin);
      TempBlueData[iterator] = getTempBlue();
      HumBlueData[iterator] = getHumBlue();
      iterator++;
      whetherGetData = false;
    }
  }
}

void CounterTime() {
  countToGetData++;
  if ( countToGetData == timeToGetData) {
    whetherGetData = true;
    countToGetData = 0;
  }
}

float getSoilHumidity(int input) {
  long wartosc_A = analogRead(input);
  float value = float(wartosc_A);
  value *= 100;
  value /= 1023;
  return value; // return in percent
}

float getMeanHumidity(int input) {
  /// find max and min value in array
  float minV = 1024.0; // max value from analog read is 1023 so 1024 is enough
  float maxV = 0; // no comment ;p
  float sum = 0.0; // sum of all data
  for (int i = 0; i < samples; i++) {
    float value = 0.0;
    if (input == SHFirst) {
      value = SoilDataFirst[i];
    } else if (input == SHSecond) {
      value = SoilDataSecond[i];
    }
    minV = min(minV, value);
    maxV = max(maxV, value);
    sum = sum + value;
  }
  /// addition all values and substraction min and max value then divide by samples-2
  sum = sum - (minV + maxV);
  sum = sum / (samples - 2);
  return sum;
}

float getTempBlack() {
  tempSensors.requestTemperatures(); // Tell the DS18B20 to get make a measurement
  float value = tempSensors.getTempCByIndex(0); // Get that temperature
  return value;
}

float getMeanTempBlack() {
  /// find max and min value in array
  float minV = 1024.0; // max temperature
  float maxV = -1024.0; // min temperature
  float sum = 0.0; // sum of all data
  for (int i = 0; i < samples; i++) {
    float value = TempBlackData[i];
    minV = min(minV, value);
    maxV = max(maxV, value);
    sum = sum + value;
  }
  /// addition all values and substraction min and max value then divide by samples-2
  sum = sum - (minV + maxV);
  sum = sum / (samples - 2);
  return sum;
}

float getLiquidLevel(int input) {
  float len = 0.0;
  long readA = analogRead(input);
  float value = float(readA);
  value -= 400; // approx 400 is minimum
  value = max(value, 0); // if read smaller than 400
  value = value / 240; // approx 640 is maximum so 640-200 for scale to <0-1>
  value = min(value, 1.0); // if something bigger then 1
  return value; // return in percent
}
float getMeanLiquidLevel() {
  /// find max and min value in array
  float minV = 1.0; // max temperature
  float maxV = 0.0; // min temperature
  float sum = 0.0; // sum of all data
  for (int i = 0; i < samples; i++) {
    float value = LiquidLevelData[i];
    minV = min(minV, value);
    maxV = max(maxV, value);
    sum = sum + value;
  }
  /// addition all values and substraction min and max value then divide by samples-2
  sum = sum - (minV + maxV);
  sum = sum / (samples - 2);
  return sum;
}

float getTempBlue() {
  float t = dht.readTemperature(); // in Celsius
  // Check if any reads failed and exit early (to try again).
  if (isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0;
  }
  return t;
}

float getMeanTempBlue() {
  /// find max and min value in array
  float minV = 1024.0; // max temperature
  float maxV = -1024.0; // min temperature
  float sum = 0.0; // sum of all data
  for (int i = 0; i < samples; i++) {
    float value = TempBlueData[i];
    minV = min(minV, value);
    maxV = max(maxV, value);
    sum = sum + value;
  }
  /// addition all values and substraction min and max value then divide by samples-2
  sum = sum - (minV + maxV);
  sum = sum / (samples - 2);
  return sum;
}


float getHumBlue() {
  float h = dht.readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) ) {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0;
  }
  return h;
}

float getMeanHumBlue() {
  /// find max and min value in array
  float minV = 1024.0; // max temperature
  float maxV = -1024.0; // min temperature
  float sum = 0.0; // sum of all data
  for (int i = 0; i < samples; i++) {
    float value = HumBlueData[i];
    minV = min(minV, value);
    maxV = max(maxV, value);
    sum = sum + value;
  }
  /// addition all values and substraction min and max value then divide by samples-2
  sum = sum - (minV + maxV);
  sum = sum / (samples - 2);
  return sum;
}

