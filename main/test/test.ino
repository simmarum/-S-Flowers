#include <SoftwareSerial.h>  //dolaczenie biblioteki do obsługi programowego interfejsu UART

#define rxPin 10
#define txPin 11

SoftwareSerial esp01(rxPin, txPin); // Inicjalizacja programowego interfejsu UART: 2-RX (polaczyc z Tx modulu ESP); 3-TX (polaczyc z Rx modulu ESP)

void setup()
{
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Serial.begin(115200); // Inicjalizacja sprzetowego interfejsu UART z prędkościa 9600 bps - do komunikacji z komputerem poprzez USB
  esp01.begin(9600); // Inicjalizacja programowego  interfejsu UART z prędkościa 9600 bps - do komunikacji z modulem ESP

  pinMode(11, OUTPUT);   //ustawienie pinu 11 jako wejście dla diody LED nr 1
  digitalWrite(11, LOW); //podanie stanu niskiego na pin 11 - dioda 1 domyślnie wyłączone

  pinMode(12, OUTPUT);   //ustawienie pinu 12 jako wejście dla diody LED nr 2
  digitalWrite(12, LOW); //podanie stanu niskiego na pin 12 - dioda 2 domyślnie wyłączone

  pinMode(13, OUTPUT);   //ustawienie pinu 13 jako wejście dla diody LED nr 3
  digitalWrite(13, LOW); //podanie stanu niskiego na pin 13 - dioda 3 domyślnie wyłączone

  //Wysyłanie komend incjalizacyjnych dla modulu ESP
  sendData("AT", 2000);
  sendData("AT+RST", 2000);             //reset modułu
  //sendData("AT+GMR", 2000);        //wersja
  sendData("AT+CWMODE=1", 2000);           //ustawienie w trybie klient
  sendData("AT+CWJAP=\"IronMaiden\",\"72e3d0d9bfa2\"", 7000);
  sendData("AT+CIFSR", 2000);
  sendData("AT+PING=\"216.58.211.142\"", 10000);
}

void loop()
{

}

/*
  Funkcja wysyłająca komendy do modułu ESP01
  Parametry:
  command - komenda do wysłania
  timeout - czas oczekiwania na odpowiedź
*/
String sendData(String command, const int timeout)
{
  String response = "";
  Serial.println("\n-----SEND-BEGIN-----" + command + "-----SEND-END-----");
  command = command + "\r\n";
  //char s[command.length()] = command.c_str();
  esp01.print("AT\r\n"); // wysłanie polecenia do ESP01

  long int time = millis();

  while ( true)
  {
    String data = "";
    while (esp01.available())
    {
      char character = esp01.read();
      Serial.print(character);
      data = data + character;
      if (character == 13) {
        Serial.print("Received: ");
        Serial.println(data);
        data = "";
      }
    }
  }

}
