// GPS 
#include <Wire.h> 
#include <TinyGPS++.h>
#define RXD0 18 
#define TXD0 19
HardwareSerial ss(0);
TinyGPSPlus gps;
double LATITUDE , LONGITUDE;
String latitude="", longitude="";
double SPEED;
String wiadomosc; 

// WhatApp oraz konfiguracja Wi-Fi
#define RXD2 16
#define TXD2 17
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include "time.h"
const char* ntpServer = "pool.ntp.org"; 
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

// SSID ORAZ HASŁO SIECI WIFI
const char* ssid = "XXXXX";
const char* password = "XXXXX";

String phoneNumber = "XXXXXXX";
String apiKey = "XXXXXX";

// Zmienna pomocniczna zliczająca ilość otrzymanych sygnałów z akceleometru 
int count =0;

// WebServer - wyświetlenie w aplikacji temperatury oraz wilgotnosci 
#include <WiFiClient.h>
#include <WebServer.h>
#include <Adafruit_AHTX0.h>
#include "index.h"  
Adafruit_AHTX0 aht;
String adcValue;
String adcValue2;
bool wypadek = false;
WebServer server(80);
//
void printLocalTime() 
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Niepowodzenie uzyskania czasu rzeczywistego");
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S"); - Format uzyskanych danych
  
   char timeHour[3];
   strftime(timeHour,3, "%H", &timeinfo);
   //Serial.println(timeHour);

   char timeMonth[9];
   strftime(timeMonth,9, "%B", &timeinfo);
   //Serial.println(timeMonth);
}
void sendMessage(String message){
  // Dane do wysłaniia żądania HTTP API
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);
  // Określenie zawartości nagłówka 
  // http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Wysłanie żądania HTPP
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Wiadomość alarmową wysłano pomyślnie");
  }
  else{
    Serial.println("ERROR - Wiadomość nie została wysłana");
    Serial.print("HTTP - odpowiedź: ");
    Serial.println(httpResponseCode);
  }
  // Zwolnienie zasobów
  http.end();
}
void setup() {
   Serial.begin(115200);
   Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); 
   // Inicjacja Wi-Fi
   WiFi.begin(ssid, password);
   Serial.println("Connecting");
   while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
   }
   Serial.println("");
   Serial.print("Połączono z siecią Wi-Fi o adresie IP: ");
   Serial.println(WiFi.localIP());
   //Inicjacja oraz uzyskanie czasu
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
   printLocalTime();
   // Inicjacja czujnika AHT20
   setup_aht(); 
   setup_WebServer();   
   // Inicjacja modułu GPS
   gps_setup();
}
void loop_sending_warning_message(){
    Serial.println(Serial2.readString());
    String dane = (Serial2.readString()); // Odczyt przesłanych danych z Arduino
    int myInt = dane.toInt(); 
   if (myInt == 9){ // Nieprawidłowe ustawienie hulajnogi 
   count++;
   //Serial.print("Count: "); Serial.println(count);
   }
   if( myInt == 10 || myInt == 4) { // W przypadku prawidłowej pozycji hulajnogi odliczanie jest zerowane
   count = 0;
   //Serial.print("Count: "); Serial.println(count);
   }
   if(count == 10 && myInt != 4){ //Wywołanie zdarzenia wysłania wiadomości alarmowej
   sendMessage("Wypadek, potrzebuje pomocy!!" + wiadomosc);
   count = 0;
   }
   if(myInt == 4) { // Poinformowanie o fałszywym alarmie 
   sendMessage("Fałszywy Alarm, wszystko w porządku!!" );
   }
}
void displayInfo()
{
   /* 
   Serial.print(F("Lokalizacja: ")); 
   if (gps.location.isValid())
   {
    /// Druk współrzednych geograficznych
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
   }
   else
   {
    Serial.print(F("INVALID"));
   }
   Serial.print(F("  Date/Time: "));
   if (gps.date.isValid())
   {
    /* Druk aktualnego czasu
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
   }
   else
   {
    Serial.print(F("INVALID"));
   } */
Serial.print(F(" "));
if (gps.time.isValid())
{
LATITUDE = gps.location.lat(), 6 ; // Uzyskanie z modułu GPS szerokości geograficznej
latitude = double_string_con(LATITUDE); 
LONGITUDE = gps.location.lng(), 6 ;
longitude = double_string_con(LONGITUDE);
SPEED = gps.speed.kmph();          
  /* Druk aktualnego czasu
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond()); */
   }
GPSinfoToMessage();
}
void GPSinfoToMessage(){
   wiadomosc = "Współrzedne geograficzne hulajnogi: ";
   wiadomosc = wiadomosc + "\n";
           
   wiadomosc = wiadomosc + "Szerokość geograficzna: ";
   wiadomosc = wiadomosc + latitude;
   wiadomosc = wiadomosc + "\n";
           
   wiadomosc = wiadomosc + "Długość geograficzna: ";
   wiadomosc = wiadomosc + longitude;
   wiadomosc = wiadomosc + "\n";
           
   wiadomosc = wiadomosc + "Aktualna prędkość: ";
   wiadomosc = wiadomosc + (String)SPEED +"m/s";
   wiadomosc = wiadomosc + "\n";  
           
   wiadomosc = wiadomosc + "Google Maps link: ";                    
   wiadomosc = wiadomosc + "\n";
   wiadomosc = wiadomosc + "https://www.google.com/maps/search/?api=1&query=";
   wiadomosc = wiadomosc + latitude;
   wiadomosc = wiadomosc + ",";
   wiadomosc = wiadomosc + longitude;
     
  delay(2000); // Opóżnienie 2 sekundy
}
void gps_setup(){
 Serial.begin(115200);
 ss.begin(9600, SERIAL_8N1, RXD0, TXD0); 
}
void gps_loop(){
  while (ss.available() > 0) // Warunek sprawdzający czy otrzymano sygnał z satelity
    if (gps.encode(ss.read()))
      displayInfo(); // Wywołanie funkcji uzyskania danych z modułu GPS

  if (millis() > 5000 && gps.charsProcessed() < 10) // Brak sygnału
  {
    Serial.println(F("Nie wykryto sygnału GPS."));
    while(true);
  } 
}
void loop() {
  //gps_loop();
  loop_sending_warning_message();
  //printLocalTime();
  server.handleClient();
  loop_aht();
  delay(200);
 }
String double_string_con(double input)
{
  String storag1 = "";
  int count=0, count2=0;
  String storag2="";
  char dot='.';
  String val_string="";
  storag2=(String)input;
  storag1= (String)(input*1000000);
  for(int i=0; i<6; i++)
    {
     if(storag2.charAt(i)==dot) break;
     count++;
    }

  for(int i=0; i<15; i++)
    {
     if(storag1.charAt(i)==dot) break;
     count2++;
    }
  
  for(int i=0; i<count2; i++)
    {
      if(i==count) 
           {
            val_string = val_string + dot ;
           }
           val_string = val_string + storag1.charAt(i);
    } 
   count=0;
   count2=0;
   return val_string;
}
//AHT 20 oraz WebServer
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
void handleADC() {
 //bool wypadek = true;
 if (wypadek == false)
 server.send(200, "text/plane","\n  Odczyty sensorów: " "\n  Temperatura: " + adcValue +" °C "+ " \n wilgotność: "+ adcValue2 +" %"); //Send ADC value only to client ajax request
 if (wypadek == true)
  server.send(200, "text/plane","!!! WYPADEK !!!"); //Send ADC value only to client ajax request
}
void setup_aht() {
   // Serial.println("Adafruit AHT10/AHT20 demo!");
   if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
   }
    Serial.println("AHT10 or AHT20 found");
}
void setup_WebServer() {
   server.on("/", handleRoot);      //This is display page
   server.on("/readADC", handleADC);//To get update of ADC Value only
   Serial.println(WiFi.localIP());  //IP address assigned to your ESP
   server.begin();                  //Start server
   //Serial.println("HTTP server started");
}
void loop_aht() {
   sensors_event_t humidity, temp;
   aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
   /* Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
   Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");*/
   adcValue = String(temp.temperature);
   adcValue2 = String(humidity.relative_humidity);
   //delay(500);
}
//AHT 20 oraz WebServer
