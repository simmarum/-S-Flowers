#include <DallasTemperature.h>
#include <OneWire.h>
#include <TimerOne.h>
#include "DHT.h"
#include "WiFiEsp.h"

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial esp(10, 11); // RX, TX
#endif

String data; // data to send to server
char server[] = "192.168.1.4"; // www.example.com
String uri = "/api/measurement_data";// our example is /esppost.php

#define second 1000000
void CounterTime(void); // every one second is call by timer
void LedLight(int x);
/// General counter time to refresh data from sensors
long iterator = 0; // actual number of sample
const int samples = 8; // how many samples is collect
int timeToGetAllData = samples * 2; // time in seconds to get all data (please make it multiple of samples
int timeToGetData = int(timeToGetAllData / samples); /// time between every sample
long countToGetData = 0; // simple counter to know when get data from sensor
bool whetherGetData = false; // condition in main to know when get data from sensor

/// Mean
float getMean(long sum);

/// Soil humidity
#define SHFirst A0
#define SHSecond A1
long SumSoilDataFirst;
long SumSoilDataSecond;
long getSoilHumidity(int input);

/// DS18B20 - temperature (small black)
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS); // this sensor use ONEWIRE connections
DallasTemperature tempSensors(&oneWire); // to make it easier
long SumTempBlackData;
long getTempBlack(void);

/// Liquid level sensor
#define LLPin A2
long SumLiquidLevelData;
long getLiquidLevel(int input);

/// DHT11 (Temperature and humidity (blue))
#define DHTPIN 4     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float SumTempBlueData;
float SumHumBlueData;
int getTempBlue();
int getHumBlue();

/// LED
#define ledPin0 7
#define ledPin1 8


char ssid[] = "1qaz"; //your ssid of wifi
char pass[] = "1qazxsw2"; // your password to wifi
int status = WL_IDLE_STATUS;
WiFiEspClient client;

void setup()
{
  LedLight(1);
  Serial.begin(115200); // begin serial in arduino IDE another than esp
  Timer1.initialize(second); //Set interrupt every 1 second
  Timer1.attachInterrupt(CounterTime); // attach out function
  tempSensors.begin(); //Start the DallasTemperature Library
  dht.begin(); // start the dht library

  esp.begin(9600); // begin communication with esp

  
  WiFi.init(&esp);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  LedLight(3);
  Serial.println("END SETUP");
}

//reset the esp8266 module
void reset() {
  esp.println("AT+RST");
  delay(2000);
  if (esp.find("OK") ) Serial.println("Module Reset");
}

// connect esp with wifi
/*void connectWifi() {
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  esp.println(cmd);
  delay(500);
  if (esp.find("OK")) {
    Serial.println("Connected!");
  }
  esp.println("AT+CIPMUX=0");
  //else {
  //  Serial.println("Cannot connect to wifi");
  //  connectWifi();
  //}
  }*/

// function to send httppost on serwer
void httppost (String data) {

  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial
  if (client.connect(server, 4000)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    Serial.println(data);
    client.println("GET /api/measurement_data?"+data+ " HTTP/1.1");
    client.println("Host: 192.168.1.4:4000");
    client.println("Authorization: Basic h5oDjJ0BKAyTbDw+rGIYInEjN4GUKpxZxJ1");
    client.println("Connection: close");
    client.println();
  }

  /*
    esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",4000");//start a TCP connection.
    if (esp.find("OK")) {
    Serial.println("TCP connection ready");
    } delay(100);
    data = "{\"verified\": true}";
    String postRequest =
    "POST " + uri + " HTTP/1.1\r\n" +
    "authorization: Basic h5oDjJ0BKAyTbDw+rGIYInEjN4GUKpxZxJ1 \r\n" +
    "accept: *" + "/" + "*\r\n" +
    "content-length: " + data.length() + "\r\n" +
    "content-type: application/json\r\n" +
    data;
    String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.
    esp.print(sendCmd);
    esp.println(postRequest.length() );
    Serial.println(postRequest);
    delay(100);
    if (esp.find(">")) {
    Serial.println("Sending..");
    esp.print(postRequest);
    if ( esp.find("SEND OK")) {
      Serial.println("Packet sent");
      while (esp.available()) {
        String tmpResp = esp.readString();
        Serial.println(tmpResp);
      }
      // close the connection
      esp.println("AT+CIPCLOSE");
    }
    }*/
}

void loop() {

  if (whetherGetData) {
    LedLight(2);
    if (iterator == samples) {
      //Serial.println("\nWysyłanie danych przez wifi");
      int meanSoilFirst = 100 - round((getMean(SumSoilDataFirst) / 1024) * 100);
      SumSoilDataFirst = 0;
      //Serial.print("\tSH1:");
      //Serial.print(meanSoilFirst);
      int meanSoilSecond = round((getMean(SumSoilDataSecond) / 1024) * 100);
      SumSoilDataSecond = 0;
      //Serial.print("\tSH2:");
      //Serial.print(meanSoilSecond);
      float meanTempBlack = round(getMean(SumTempBlackData) / 10) / 10;
      SumTempBlackData = 0;
      //Serial.print("\tTBk:");
      //Serial.print(meanTempBlack);
      int meanLiquidLevel = round(getMean(SumLiquidLevelData));
      SumLiquidLevelData = 0;
      //Serial.print("\tLL:");
      //Serial.print(meanLiquidLevel);
      float meanTempBlue = round(getMean(SumTempBlueData) / 10) / 10;
      SumTempBlueData = 0;
      // Serial.print("\tTBe:");
      //Serial.print(meanTempBlue);
      int meanHumBlue = round(getMean(SumHumBlueData) / 100);
      SumHumBlueData = 0;
      //Serial.print("\tHBe:");
      //Serial.print(meanHumBlue);
      //Serial.println();
      // fill data to send
      data = "soil_humidity=" + String(meanSoilFirst) + "&" // INT -> percentage
      //    + "\"soil2\":" + String(meanSoilSecond)  + ","// INT -> percentage
         + "air_temperature=" + String(meanTempBlack) + "&" // FLOAT (XX.XX) -> Celsius degrees
        // + "air_humidity=" + String(meanTempBlue)  + "&"// FLOAT (XX.XX) -> Celsius degrees
         + "liquid_level_millimeters=" + String(meanLiquidLevel) + "&" // INT -> percentage (exact mm is impossible because documentation of this sensor is very different from this what sensor is response ;/)
      // on server make something like "low level", "medium level", "high level" of water?
         + "air_humidity=" + String(meanHumBlue); //INT -> percentage
      httppost(data); // send data
      // reset all variable after send data
      iterator = 0;
      countToGetData = 0;
      whetherGetData = false;
    } else {
      // get one sample of data from sensor
      Serial.print("Pobieram próbkę numer: ");
      Serial.println(iterator + 1);
      SumSoilDataFirst += getSoilHumidity(SHFirst);
      SumSoilDataSecond += getSoilHumidity(SHSecond);
      SumTempBlackData += getTempBlack();
      SumLiquidLevelData += getLiquidLevel(LLPin);
      SumTempBlueData += getTempBlue();
      SumHumBlueData += getHumBlue();
      iterator++;
      whetherGetData = false;
    }
    LedLight(3);
  }
}

void CounterTime() {
  countToGetData++;
  if ( countToGetData == timeToGetData) {
    whetherGetData = true;
    countToGetData = 0;
  }
}

// DEBUG - function to light LED in binary mode (2 bit)
void LedLight(int x) {
  if (x > 3 or x < 0) x = 0;
  int a0 = x % 2;
  int a1 = x / 2;
  digitalWrite(ledPin1, a0);
  digitalWrite(ledPin0, a1);
}

// return mean of samples in float
float getMean(long sum) {
  float sum_f = (float)sum;
  return sum_f / samples;
}


long getSoilHumidity(int input) {
  long soil = analogRead(input);
  return soil;
}


long getTempBlack() {
  tempSensors.requestTemperatures(); // Tell the DS18B20 to get make a measurement
  float value = tempSensors.getTempCByIndex(0); // Get that temperature
  long valueRet = value * 100;
  return valueRet;
}

long getLiquidLevel(int input) {
  long readA = analogRead(input);
  float value = float(readA);
  value -= 400; // approx 400 is minimum
  value = max(value, 0); // if read smaller than 400
  value = value / 240; // approx 640 is maximum so 640-200 for scale to <0-1>
  value = min(value, 1.0); // if something bigger then 1
  readA = value * 100; // from <0-1> to <0-100> and int not float
  return value; // return in percent
}

int getTempBlue() {
  float t = dht.readTemperature(); // in Celsius
  // Check if any reads failed and exit early (to try again).
  if (isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0;
  }
  int tem = t * 100;
  return tem;
}

int getHumBlue() {
  float h = dht.readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) ) {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0;
  }
  int hum = h * 100;
  return hum;
}

