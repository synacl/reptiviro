/*************************************************** 
Reptile cage environmental monitor
 ****************************************************/
#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h>  // RTC 
#include <LCD4Bit_mod.h> //4bit LCD Shield w/ buttons
#include <dht.h> // Digital Humidity and Temp
dht DHT;
LCD4Bit_mod lcd = LCD4Bit_mod(2); // 2 line display

#define DHT22_PIN 50 //Ambient Temp and Humidity
#define TMP36 10    // Surface Temp for UTH control
#define HEATPAD 22  // UTH Relay output
#define REDPIN 40   // 5050 LED Strip
#define BLUEPIN 42  // 5050 LED Strip
#define GREENPIN 44 // 5050 LED Strip

/* Day Start and End Hour (24hr format) */
int day_start = 8;
int day_end = 19; 
/* Surface temps wanted (Celcius) */
float target_NightC = 30;
float target_DayC = 31;

// Sensor poll and Display update interval
const long INTERVAL = 5000; //  in mills (1000 = 1sec)
unsigned long previousMillis = 0;

float target_tempC; 
float target_tempF;
float c;
float f;
char mode[6];

void setup() {
  /* IO setup */
 // pinMode(ONBOARDLED, OUTPUT);
  //analogReference(EXTERNAL);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(HEATPAD, OUTPUT);
  lcd.init();
  lcd.clear();
  Serial.begin(9600);
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time"); 
}

void loop() {
  // Check Day/Time
  if (hour() >= day_start && hour() < day_end){
    sprintf(mode,"Day");
    target_tempC = target_DayC; // Celcuis
    Daylight();
  }
  else {
    // Night Cycle
    sprintf(mode,"Night");
    target_tempC = target_NightC; // Celcius
    Nightlight();
  }
  // read sensors and update display
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    getTemp();
    getHumidity();
    updateDisplay();
  } 
}
void getTemp(){
  // get current
  int tmp36read = analogRead(TMP36);
  float tmp36mv = ((tmp36read * 4.75) / 1000.0);
  c = ((tmp36mv - .5) * 100);
  f = ( c * (9.0 / 5.00)) + 32;
  if (isnan(c)) {
    //sprintf(topmsg,"Sensor Fault");
    //lcd.cursorTo(1, 0);
    //lcd.printIn(topmsg);
    Serial.print("TMP36 Sensor Fault");
  } 
  else {
    if (c < target_tempC){
      digitalWrite(HEATPAD, HIGH);
    }
    else{
      digitalWrite(HEATPAD, LOW);
    }    
  }
}

void getHumidity(){
  // Get Humidity  
  int chk = DHT.read22(DHT22_PIN);
  if(chk != DHTLIB_OK)
  {
		Serial.print("DHT22 Sensor Fault"); 
  }
}
  
void updateDisplay(){  
  // DISPLAY DATA
  target_tempF = (target_tempC * (9.0 / 5.0)) + 32;
  char ttempstr[6];
  char topmsg[17]; //top lcd line buffer
  dtostrf(target_tempF,3,0,ttempstr);
  sprintf(topmsg,"%02d:%02d %s:%sF",hour(),minute(),mode,ttempstr);
  //Serial.print(topmsg);
  lcd.cursorTo(1,0);
  lcd.printIn(topmsg);
  float dhtf = ( DHT.temperature * (9.0 / 5.0)) + 32;
  lcd.cursorTo(2, 0);  //line=2, x=0
  char bmsg[17];
  char fstr[5]; //tmp36
  char dhtH[4]; //dht22 Humidity
  char dhtF[5]; //dht22 Temp F
  dtostrf(f,3,0, fstr);
  dtostrf(DHT.humidity,2,0,dhtH);
  dtostrf(dhtf,3,0,dhtF);
  sprintf(bmsg,"U%sF A%sF %s%%H",fstr,dhtF,dhtH);
  lcd.printIn(bmsg);
}  
  
void Daylight(){
  analogWrite(GREENPIN,100);
  analogWrite(REDPIN, 150);
  analogWrite(BLUEPIN, 100);
}

void Nightlight(){
  analogWrite(REDPIN, 128);
  analogWrite(BLUEPIN, 128);
}

  
