
#include <ESP8266WiFi.h>
#include <GoogleMapsApi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WifiLocation.h>

char ssid[] = "XXX";       
char password[] = "XXX";  
#define API_KEY "XXX"  
#define ARDUINO_MAPS_BOT_TOKEN "5922002888:AAFC_tcGywb1cIC5ckKTTzgLJ5MiXesEyfA" //Telegram token
#define CHAT_ID "5922002888" //Telegram chat ID

WiFiClientSecure client;
GoogleMapsApi api(API_KEY, client);
UniversalTelegramBot bot(ARDUINO_MAPS_BOT_TOKEN, client);
WifiLocation location(API_KEY);


// Wartości domyślne dla zmiennych zdefiniowanych w biblotece "GoogleMapsApi"
int api_mtbs = 60000; // czas pomiędzy żądaniami API
long api_lasttime = 0;   // Ostatnie wykonane żądanie API
bool firstTime = true;

//Inputs
String origin = "Warsaw, Poland";
String destination = "Piaseczno, Poland";
String current_location;
String dane;

//Zmienne do wywoływania działania nawigacji oraz pobierania aktualnej pozycji użytkownika
bool flag1 = 0;
bool flag2 = 0;
bool flag3 = 0;

// Dodatkowe opcje Distance Matrix API
String departureTime = "now"; 
String travelMode = "driving";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // STA - tryb stacji roboczej
  WiFi.disconnect();
  delay(100);
  Serial.print("Łączenie z siecia Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    client.setInsecure();
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Połączono z siecią Wi-Fi");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  //get_current_GEOLOCATION(); - Wywołanie pobrania aktualnej lokalizacji z żądania GEOLocation API
}


void handleNewMessages(int newMessage)
{
  Serial.print("Otrzymana nowa wiadomość z komunikatora Telegram:  ");
  Serial.println(newMessage);

  for (int i = 0; i < newMessage; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;


     if (text.startsWith("/Koniec")) {
        Serial.println(text);
        text.remove(0, 8);
        destination = text;
        bot.sendMessage(chat_id, "Miejsce docelowe: " + String(text), "");
        flag1 = 1;
      }
     if (text.startsWith("/Start")) {
        Serial.println(text);
        text.remove(0, 7);
        origin = text;
        bot.sendMessage(chat_id, "Miejsce rozpoczęcia trasy: " + String(text), "");
        flag2 = 1;
      }
     if (text =="/GEOStart") {
        bot.sendMessage(chat_id, "Miejsce rozpoczęcia trasy(aktualna lokalizacja): " + String(current_location), "");
        origin = current_location;
        flag2 = 1;
      }
           if (text =="/Pozycja") {
        flag3 = 1;
        bot.sendMessage(chat_id, "Trwa pozyskiwanie aktualnej geolokalizacji ");
      }
      if (text =="/Wyswietl") {
       bot.sendMessage(chat_id, "Aktualne współrzedne geograficzne: " + String(current_location), "");
      }
           if (text =="/Nawigacja") {
     bot.sendMessage(chat_id,dane);
           }
     if (text == "/Stop") {
        flag1 = 0, flag2= 0;
        bot.sendMessage(chat_id, "Zatrzymanie działania nawigacji");
      }

    if (text == "/Info")
    {
      String INFOMESSSAGE = "Nawigacja GoogleMapsAPI - ESP8266\n\n";
      INFOMESSSAGE += "Poniższymi komendami określasz położenie startowe, docelowe, wyświetlasz dane nawigacyjne oraz zatrzymujesz działania nawigacji.\n\n";
      INFOMESSSAGE += "/Start : Ustalenie początku miejsca trasy\n";
      INFOMESSSAGE += "/Koniec : Ustalenie końca miejsca trasy\n";
      INFOMESSSAGE += "/Nawigacja : Wyświetlenie danych nawigacyjnych\n"; 
      INFOMESSSAGE += "/Pozycja : Uzyskanie aktualnej geookalizacji\n"; 
      INFOMESSSAGE += "/Wyswietl : Wyświetlenie aktualnej geolokalizacji\n";      
      INFOMESSSAGE += "/GEOStart : Start trasy z aktualnej pozycji\n";
      INFOMESSSAGE += "/Stop : Zatrzymanie działania nawigacji\n";
      bot.sendMessage(chat_id, INFOMESSSAGE);
    }


if (text == "/Rower") travelMode = "bicycling";  
      
if (text == "/Auto")  travelMode = "driving";  
      
if (text == "/Pieszo") travelMode = "walking";  

if (text == "/Komunikacja") travelMode = "transit";  
      

if(flag3 == true) {
  get_current_GEOLOCATION();
  flag3 = 0;
  }
if(flag1 == true && flag2 == true){
  checkGoogleMaps();
  flag1 = 0; 
 } 
}
}

void get_current_GEOLOCATION(){
 setClock(); // Pobranie aktualne czasu rzeczywistego
 location_t loc = location.getGeoFromWiFi(); // Uzyskanie współrzednych geograficznych
 current_location = String(loc.lat, 7) + ","+String(loc.lon, 7); // zapis uzyskanych danych
 }

void setClock () {
    configTime (0, 0, "pool.ntp.org", "time.nist.gov");
   // Serial.print ("Waiting for NTP time sync: ");
    time_t now = time (nullptr);
    while (now < 8 * 3600 * 2) {
        delay (500);
        Serial.print (".");
        now = time (nullptr);
    }
    struct tm timeinfo;
    gmtime_r (&now, &timeinfo);
   // Serial.print ("\n");
   // Serial.print ("Current time: ");
   // Serial.print (asctime (&timeinfo));
}

bool checkGoogleMaps() {
    Serial.println("Uzyskanie nawigacji z: " + origin + " do: " + destination);
    String responseString = api.distanceMatrix(origin, destination, departureTime, travelMode);
    DynamicJsonDocument jsonBuffer(1024);
    DeserializationError response = deserializeJson(jsonBuffer, responseString);
    JsonObject root = jsonBuffer.as<JsonObject>();

    if (!response) 
    {
       String status = root["status"];
        if(status =="OK") 
        {
          Serial.println("Status : " + status);
          String durationInTraffic = root["rows"][0]["elements"][0]["duration_in_traffic"]["text"];
          int distanceInKm = root["rows"][0]["elements"][0]["distance"]["value"];
          distanceInKm = distanceInKm/1000;
          dane = "Odległość pomiędzy lokalizacjami: "+ String(origin) +" i " + String(destination) + + " "+ String(distanceInKm) +" km" +" Czas podróży: " + String(durationInTraffic);
         
          return true;
        }
        else 
        {
          Serial.println("error status");
          return false;
        }
    }
}

void loop() {
  
    int newMessage = bot.getUpdates(bot.last_message_received + 1);
    while (newMessage)
    {
      Serial.println("Otrzymano wiadomość z aplikacji Telegram: ");
      handleNewMessages(newMessage);
      newMessage = bot.getUpdates(bot.last_message_received + 1);
    }

}
