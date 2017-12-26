#include <SoftwareSerial.h>  //dolaczenie biblioteki do obsługi programowego interfejsu UART
         
SoftwareSerial esp01(10,11); // Inicjalizacja programowego interfejsu UART: 2-RX (polaczyc z Tx modulu ESP); 3-TX (polaczyc z Rx modulu ESP)
     
void setup()
{
  Serial.begin(9600); // Inicjalizacja sprzetowego interfejsu UART z prędkościa 9600 bps - do komunikacji z komputerem poprzez USB
      esp01.begin(9600); // Inicjalizacja programowego  interfejsu UART z prędkościa 9600 bps - do komunikacji z modulem ESP
       
  pinMode(11,OUTPUT);    //ustawienie pinu 11 jako wejście dla diody LED nr 1
  digitalWrite(11,LOW);  //podanie stanu niskiego na pin 11 - dioda 1 domyślnie wyłączone
       
  pinMode(12,OUTPUT);    //ustawienie pinu 12 jako wejście dla diody LED nr 2
  digitalWrite(12,LOW);  //podanie stanu niskiego na pin 12 - dioda 2 domyślnie wyłączone
       
  pinMode(13,OUTPUT);    //ustawienie pinu 13 jako wejście dla diody LED nr 3
  digitalWrite(13,LOW);  //podanie stanu niskiego na pin 13 - dioda 3 domyślnie wyłączone
        
  //Wysyłanie komend incjalizacyjnych dla modulu ESP
  sendData("AT+RST",2000);              //reset modułu
  sendData("AT+GMR",1000);         //wersja
  sendData("AT+CWMODE=1",1000);            //ustawienie w trybie klient
  sendData("AT+CWJAP=\"IronMaiden\",\"72e3d0d9bfa2\"",7000);
  sendData("AT+CIFSR",2000);
  sendData("AT+PING=\"216.58.211.142\"",10000);
}
      
void loop()
{

}
      
/*
* Funkcja wysyłająca komendy do modułu ESP01
* Parametry:
* command - komenda do wysłania
* timeout - czas oczekiwania na odpowiedź
*/
String sendData(String command, const int timeout)
{
    String response = "";
    command = command + "\r\n";     
    Serial.println("-----SEND-BEGIN-----" + command + "-----SEND-END-----");
    esp01.print(command); // wysłanie polecenia do ESP01
         
    long int time = millis();
         
    while( (time+timeout) > millis())
    {
      while(esp01.available()) //jeśli są jakieś dane z modułu, wtedy następuje ich odczyt
      {
        char c = esp01.read(); // odczyt kolejnego znaku
        response += c;
      }
    }
    Serial.print(response);
    Serial.println("-----END-RESPONSE-----");
    return response;
}
