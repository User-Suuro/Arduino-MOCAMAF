#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const long eventTime_1_updateLCD = 1000;
const long eventTime_2_pushButtons = 350;
const long eventTime_3_read_DHT11 = 2000;
const long eventTime_4_pushUpdate = 250;

unsigned long previousTime_1 = 0;
unsigned long previousTime_2 = 0;
unsigned long previousTime_3 = 0;
unsigned long previousTime_4 = 0;

float h = 0;
float t = 0;
float hic = 0;

// TO DO:
//automated time (asynchronous)

//relay guide: HIGH - OFF : LOW - ON
// SDA - pin A4
// SCL - pin A5
LiquidCrystal_I2C lcd(0x27, 16, 2);
int minTemp = 29;
int maxTemp = 31;

int relayPin01 = 7;
int relayPin02 = 8;
int relayPin03 = 9;

int buttonPin1 = 3;
int buttonPin2 = 4;
int buttonPin3 = 5;

int buttonVal1 = 0;
int buttonVal2 = 0;

int mode = 0;
int modeValue = 0;

void setup() {
  pinMode(relayPin01, OUTPUT);
  pinMode(relayPin02, OUTPUT);
  pinMode(relayPin03, OUTPUT);

  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  lcd.clear();
	lcd.begin();
	lcd.backlight();
  Serial.begin(9600);
  dht.begin();
  delay(2000);
}



void loop() {
   unsigned long currentTime = millis();

  if (currentTime - previousTime_3 >= eventTime_3_read_DHT11){
    h = dht.readHumidity();
    t = dht.readTemperature();
    hic = dht.computeHeatIndex(t, h, false);
    previousTime_3 = currentTime;
  }

  //relay check
  if (t <= minTemp){
    digitalWrite(relayPin01, HIGH);
    digitalWrite(relayPin02, HIGH);
    digitalWrite(relayPin03, LOW);
  }else if (t >= maxTemp){
    digitalWrite(relayPin01, LOW);
    digitalWrite(relayPin02, LOW);
    digitalWrite(relayPin03, HIGH);
  }else{
    digitalWrite(relayPin01, LOW);
    digitalWrite(relayPin02, LOW);
    digitalWrite(relayPin03, LOW);
  };
  ///////////////////////////////////

  //buttons//
  if (currentTime - previousTime_2 >= eventTime_2_pushButtons){
    modeValue = digitalRead(buttonPin2);
    //adjust min temp
    if (mode == 1 || 2){
      buttonVal1 = digitalRead(buttonPin1);
      buttonVal2 = digitalRead(buttonPin3);
    }

    previousTime_2 = currentTime;
  }
  if (currentTime - previousTime_4 >= eventTime_4_pushUpdate){
    if (mode == 1){
       if (buttonVal1 != 0){
          minTemp--;
          lcd.setCursor(6, 1);
          lcd.print(minTemp);   
        };
      if (buttonVal2 != 0){
        if (minTemp > maxTemp - 2){ //set limit for adjusting min temp
          return;
        }
        minTemp++;
        lcd.setCursor(6, 1);
        lcd.print(minTemp); 
      }
    }else if (mode == 2){
      if (buttonVal1 != 0){
        if (minTemp > maxTemp - 2){ //set limit for adjusting max temp
          return;
        }
        maxTemp--;
        lcd.setCursor(6, 1);
        lcd.print(maxTemp); 
        };
        if (buttonVal2 != 0){
          maxTemp++;
          lcd.setCursor(6, 1);
          lcd.print(maxTemp); 
        };
    }
    
    previousTime_4 = currentTime;
  }
  if(modeValue != 0){
      mode++;
      modeValue = 0;
      lcd.clear();
    
    if (mode > 2){ //set limit mode number
      mode = 0;
    }
    return;
  }else{ //will keep running depending on mode
    if (mode == 0){
      //catch
      if (isnan(h) || isnan(t)) {
        lcd.print("Failed to Detect");
        Serial.println(F("Failed to read from DHT sensor!"));
      return;}

      //otherwise display
      if (currentTime - previousTime_1 >= eventTime_1_updateLCD){
        lcd.clear();
        previousTime_1 = currentTime;
        lcd.print("Temp: ");
        lcd.print(t);
        lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Humid: ");
        lcd.print(h);
      }
    }
   else if (mode == 1){
     
     if (currentTime - previousTime_1 >= eventTime_1_updateLCD){
      lcd.clear();
      lcd.print("Adjust Minimum");
      lcd.setCursor(0, 1);
      lcd.print("Temp: ");
      previousTime_1 = currentTime;
      lcd.print(minTemp);
    }
    }else if (mode == 2){
      if (currentTime - previousTime_1 >= eventTime_1_updateLCD){
        lcd.clear();
        lcd.print("Adjust Maximum");
        lcd.setCursor(0, 1);
        lcd.print("Temp: ");
        previousTime_1 = currentTime; 
        lcd.print(maxTemp);    
        return;
      }
    }
  };
};




 



