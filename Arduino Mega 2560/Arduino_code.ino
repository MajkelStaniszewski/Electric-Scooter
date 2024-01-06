#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_ADXL345_U.h>
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

// Zasilanie
#define backlightUp 38
#define backlightDown 39
#define frontlightUp 40
#define frontlightDown 41
#define gasUp 3
#define gasDown 2
#define blueUp 4
#define leftUp 36
#define leftDown 37
#define rightUp 34
#define rightDown 35
#define buzzerUp 50
#define buzzerDown 51
#define servoUp 6
#define servoDown 5
#define GPSUp 9
#define GPSDown 8
//Servo
#include <Servo.h>
int pos=random(0,180);
Servo servo;
//Czujnik nacisku + Światło tylne
#define DELAYVAL 1000
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define TYLNE_SWIATLO        22
#define NUMPIXELS 8
Adafruit_NeoPixel swiatlo_tylne(NUMPIXELS, TYLNE_SWIATLO, NEO_GRB + NEO_KHZ800);
int pressureAnalogPin = A1; //pin where our pressure pad is located.
float pressureReading; //variable for storing our reading
float pressure_max = 60;

//Kierunkowskaz przedni + Światła nocne
bool bool_miganie = true;
bool bool_skrecanie = false;
#define PRZEDNIE_SWIATLO   23
Adafruit_NeoPixel swiatlo_przednie(NUMPIXELS, PRZEDNIE_SWIATLO, NEO_GRB + NEO_KHZ800);
bool night = false;
bool nightlightApproved=false;
bool blinking = false;
#define ACTIVATED LOW
const int switch_lewy_pin = 30;
int lewy_state = 0;
const int switch_prawy_pin = 31;
int prawy_state = 0;
//
#define PIN_buzzer  32
bool lightsOn=false;
//hc06
char Incoming_value = 'X';
SoftwareSerial mySerial(53,52); // RX, TX   
//MQ135 - gas sensor
#include <MQ2.h>
int Analog_Input = A0;
int smoke;
MQ2 mq2(Analog_Input);
//int lpg, co,value;
void setup_power(){
  pinMode(servoUp,OUTPUT);
  pinMode(servoDown,OUTPUT);
  pinMode(buzzerUp,OUTPUT);
  pinMode(buzzerDown,OUTPUT);
  pinMode(blueUp,OUTPUT);
  pinMode(leftUp,OUTPUT);
  pinMode(leftDown,OUTPUT);
  pinMode(rightUp,OUTPUT);
  pinMode(rightDown,OUTPUT);
  pinMode(gasUp,OUTPUT);
  pinMode(gasDown,OUTPUT);
  pinMode(frontlightUp,OUTPUT);
  pinMode(frontlightDown,OUTPUT);
  pinMode(backlightUp,OUTPUT);
  pinMode(backlightDown,OUTPUT);
  pinMode(GPSUp,OUTPUT);
  pinMode(GPSDown,OUTPUT);
  digitalWrite(servoUp,HIGH);
  digitalWrite(servoDown,LOW);
  digitalWrite(buzzerUp,HIGH);
  digitalWrite(buzzerDown,LOW);
  digitalWrite(blueUp,HIGH);
  digitalWrite(leftUp,HIGH);
  digitalWrite(leftDown,LOW);
  digitalWrite(rightUp,HIGH);
  digitalWrite(rightDown,LOW);
  digitalWrite(gasUp,HIGH);
  digitalWrite(gasDown,LOW);
  digitalWrite(frontlightUp,HIGH);
  digitalWrite(frontlightDown,LOW);
  digitalWrite(backlightUp,HIGH);
  digitalWrite(backlightDown,LOW);
  digitalWrite(GPSUp,HIGH);
  digitalWrite(GPSDown,LOW);
}
void setup_MQ135(){
  mq2.begin();
}
void setup_switche(){
 pinMode(switch_lewy_pin, INPUT);
 digitalWrite(switch_lewy_pin,HIGH);
 pinMode(switch_prawy_pin, INPUT);
 digitalWrite(switch_prawy_pin,HIGH);
}
void setup_czujnik_nacisku(){
 #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
 #endif
  swiatlo_tylne.begin();
}
void setup_buzzer(){
  pinMode(PIN_buzzer,OUTPUT);
}
void setup_swiatlo_nocne(){
  swiatlo_przednie.begin();  
}
void setup_servo(){
     servo.attach(7);
}
void setup_hc06(){
  mySerial.begin(9600);
}
void setup_akcel(){
 if(!accel.begin())
   {
      Serial.println("No ADXL345 sensor detected.");
      while(1);
   }
   else
   Serial.println("ADXL345 sensor detected.");    
}
int hc06_loop() {
  if(mySerial.available()> 0 )  
  {
    Incoming_value = mySerial.read();      
    Serial.print(Incoming_value);                 
  /*  LED TEST 
   *   Serial.print("\n");
   if(Incoming_value == '1')             
      digitalWrite(13, HIGH);  
    else if(Incoming_value == '0')       
      digitalWrite(13, LOW);  */    
  }      
}
void buzzerPlay_loop(){
  digitalWrite(PIN_buzzer, HIGH);
  delay(0.1*DELAYVAL);
  digitalWrite(PIN_buzzer, LOW);
  delay(DELAYVAL);  
}
void buzzerStop_loop(){
  digitalWrite(PIN_buzzer, LOW);
}
void upadek_loop(){
    sensors_event_t event; 
   accel.getEvent(&event);
  /* Druk informacji z akceleometru
   Serial.println("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
   Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
   Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");
  */
   float y = event.acceleration.y;
if(lightsOn==false){   
   if (!( (y > 7 && y <12) || (y < -7 && y > -12) )){
   delay(DELAYVAL);
   Serial.println("9");
   }
   if (( (y > 7 && y <12) || (y < -7 && y > -12) )){
   delay(DELAYVAL);
   Serial.println("10");
   }
}  
}
void czujnik_nacisku_loop(){
  pressureReading = analogRead(pressureAnalogPin);
   swiatlo_tylne.clear();
     Serial.print("Pressure Pad Reading = ");
     Serial.println(pressureReading);

float pressure_light = (pressureReading/pressure_max)*255;
      for(int i=0; i<NUMPIXELS; i++) {
    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(pressure_light, 0, 0));
    swiatlo_tylne.show();
}
}
void MQ135_loop(){
   float* values= mq2.read(false); //set it false if you don't want to print the values in the Serial
   smoke = mq2.readSmoke();
   //Serial.println("SMOKE:");
   Serial.println(smoke);
   if (smoke > 6900){
      buzzerPlay_loop();
      bool_miganie = true;
      if(bool_miganie){
        wlacz_swiatla_awaryjne();
        delay(0.2*DELAYVAL);
        wylacz_swiatla_awaryjne();
        delay(0.1*DELAYVAL);
      } 
      }
}
void blokada(){
    //Serial.println("Krecimy");
    servo.write(180);
    delay(DELAYVAL);
    
}
void wlacz_swiatla_stopu(){
  swiatlo_tylne.clear();
       for(int i=0; i<NUMPIXELS; i++) {

    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(255, 0, 0));
    swiatlo_tylne.show();
}
}
void wylacz_swiatla_stopu(){
  swiatlo_tylne.clear();
        for(int i=0; i<NUMPIXELS; i++) {
    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(0, 0, 0));
    swiatlo_tylne.show();
}   
}
void wlacz_swiatla_awaryjne(){
  swiatlo_tylne.clear();
  swiatlo_przednie.clear(); 
    for(int i=0; i<NUMPIXELS; i++) {
    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(255, 255, 0));
    swiatlo_tylne.show();
  }
    for(int i=NUMPIXELS;i>=0;i--){
    swiatlo_przednie.setPixelColor(i, swiatlo_przednie.Color(255, 255, 0));
    swiatlo_przednie.show(); 
  }  
}
void wlacz_swiatla_przednie(){  
    swiatlo_przednie.clear();
    for(int i=NUMPIXELS;i>=0;i--){
    swiatlo_przednie.setPixelColor(i, swiatlo_przednie.Color(255, 255, 255));
    swiatlo_przednie.show();
  }  
}
void wylacz_swiatla_przednie(){  
    swiatlo_przednie.clear();
    for(int i=NUMPIXELS;i>=0;i--){
    swiatlo_przednie.setPixelColor(i, swiatlo_przednie.Color(0, 0, 0));
    swiatlo_przednie.show();  
  }  
}
void wylacz_swiatla_awaryjne(){
  swiatlo_tylne.clear();
  swiatlo_przednie.clear(); 
    for(int i=0; i<NUMPIXELS; i++) {
    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(0, 0, 0));
    swiatlo_tylne.show();
  }
    for(int i=NUMPIXELS;i>=0;i--){
    swiatlo_przednie.setPixelColor(i, swiatlo_przednie.Color(0, 0, 0));
    swiatlo_przednie.show();
  }
}
void switche_loop(){
  lewy_state = digitalRead(switch_lewy_pin);
   //  Serial.println(lewy_state);  
  prawy_state = digitalRead(switch_prawy_pin);
   //  Serial.println(switch_prawy_pin);  
}
void kierunkowskaz(){
  switche_loop();
   if(blinking==true){
   if (prawy_state == ACTIVATED){
    bool_skrecanie = true;
    for(int i=((NUMPIXELS/2)-1); i>=0; i--) {
    swiatlo_przednie.setPixelColor(i, swiatlo_przednie.Color(255, 255, 0));
    swiatlo_przednie.show();
    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(255, 255, 0));
    swiatlo_tylne.show(); 
    delay(0.6*DELAYVAL);
   }
   blinking = false;
   }
   else if (lewy_state == ACTIVATED){
   bool_skrecanie = true;
    for(int i=NUMPIXELS/2;i<=NUMPIXELS;i++){
    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(255, 255, 0));
    swiatlo_tylne.show();
    swiatlo_przednie.setPixelColor(i, swiatlo_przednie.Color(255, 255, 0));
    swiatlo_przednie.show();
    delay(0.6*DELAYVAL);
   }
   blinking = false;
   }
   else{
  bool_skrecanie = false;  
    for(int i=0; i<NUMPIXELS; i++) {
    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(0,0,0));
    swiatlo_tylne.show(); 
    swiatlo_przednie.setPixelColor(i, swiatlo_przednie.Color(0,0,0));
    swiatlo_przednie.show();
   }
   }
   }
   else{
   for(int i=0; i<NUMPIXELS; i++) {
    bool_skrecanie = true;
    swiatlo_tylne.setPixelColor(i, swiatlo_tylne.Color(0,0,0));
    swiatlo_tylne.show();  
    swiatlo_przednie.setPixelColor(i, swiatlo_przednie.Color(0,0,0));
    swiatlo_przednie.show();
   }
   delay(0.1*DELAYVAL);
   blinking = true;  
   }
}
void setup() 
{
   Serial.begin(9600);
   setup_power();
   setup_czujnik_nacisku();
   setup_swiatlo_nocne();
   setup_hc06(); 
   setup_akcel(); 
   setup_servo();
   setup_switche();
   setup_buzzer();
   setup_MQ135();
  
}
void loop(){ 
   hc06_loop();
   czujnik_nacisku_loop();
   // ograniczenie opóźnień
   if(pressureReading == 0 || Incoming_value != 'a' || Incoming_value != 'c' || Incoming_value != 'e'){
     lightsOn=true;
   }
   else
   lightsOn=false;
   upadek_loop();
   kierunkowskaz();
   MQ135_loop();
// Wysunięcie popychacza
if(Incoming_value == '0'){
blokada();
}
// Powrót popychacza 
if(Incoming_value != '0'){
servo.write(90);
delay(DELAYVAL);
}    
// Alarm  
if(Incoming_value == '1'){
      buzzerPlay_loop();
      bool_miganie = true;
      if(bool_miganie){
        wlacz_swiatla_awaryjne();
        delay(0.2*DELAYVAL);
        wylacz_swiatla_awaryjne();
        delay(0.1*DELAYVAL);
      }   
}
// Wylacz blokade
if(Incoming_value == '2'){       
}
// Wylacz alarm
if(Incoming_value == '3'){
      buzzerStop_loop();
      bool_miganie = false;  
      wylacz_swiatla_awaryjne(); 
}
// Odwołanie wiadomości alarmowej
// Incoming_value == '4'

if(bool_skrecanie == false){
// Włącz światła przednie
if(Incoming_value == 'a'){
  wlacz_swiatla_przednie();
}
//Wyłącz światłą przednie
if(Incoming_value == 'b'){
  wylacz_swiatla_przednie();    
}
// Włącz światła stopu
if(Incoming_value == 'c'){
  wlacz_swiatla_stopu(); 
}
//Wyłącz światłą stopu
if(Incoming_value == 'd'){
wylacz_swiatla_stopu();      
}
// Włącz światła AWARYJNE
if(Incoming_value == 'e'){
   bool_miganie = true;
  if(bool_miganie){
wlacz_swiatla_awaryjne();
delay(0.2*DELAYVAL);
wylacz_swiatla_awaryjne();
delay(0.1*DELAYVAL);
  } 
}
//Wyłącz światła AWARYJNE
if(Incoming_value == 'f'){
bool_miganie = false;  
wylacz_swiatla_awaryjne();    
}
// Włącz światła nocne
if(Incoming_value == '*'){
       night=true;
}
if(Incoming_value == 'g'){
       nightlightApproved=true;
}
if(night==true && nightlightApproved==true){
  wlacz_swiatla_przednie();
}
//Wyłącz światła nocne
if(Incoming_value == 'h' ){
        nightlightApproved=false;
}
if(Incoming_value == '#'){
       night=false;
}
if(!(night==true && nightlightApproved==true)){
  wylacz_swiatla_przednie();
}
}
  //delay(0.6*DELAYVAL);
}



