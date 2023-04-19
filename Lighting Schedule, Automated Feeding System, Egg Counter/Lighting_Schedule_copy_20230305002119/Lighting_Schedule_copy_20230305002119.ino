#include "Arduino.h"
#include "uRTCLib.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "uEEPROMLib.h"
#include <Servo.h>

// uRTCLib rtc;
uRTCLib rtc(0x68);
uEEPROMLib eeprom(0x57);
unsigned int addr_EEPROM;

//servos
Servo myservo01;
Servo myservo02; 
Servo myservo03;

int servoPos = 0;
int posTest = 0;

//LED Display
LiquidCrystal_I2C lcd(0x27, 16, 2);

//counter
int IR_sensor = 0; // is IR ACTIVE??
int egg_count = 0;
int state = 0;
int day_count = 0;
int see_day = 0;
int egg_count_record_day[400]; // till day 1000 (memory)


const char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec"}; 
const char* days[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

//timed Events
const long eventTime_1_updateLCD = 1000; // interval in ms
const long eventTime_2_pushButtons = 250;
const long eventTime_3_readIR = 0;
const long eventTime_4_updateLCD2 = 250;
const long eventTime_5_moveServo = 15;
const long eventTime_6_write_EEPROM = 1000;

unsigned long previousTime_1 = 0;
unsigned long previousTime_2 = 0;
unsigned long previousTime_3 = 0;
unsigned long previousTime_4 = 0;
unsigned long previousTime_5 = 0;
unsigned long previousTime_6 = 0;

bool adjust_OFF = false;
bool adjust_ON = false;

bool adjust_feeding_time = false;
bool adjust_X_feeding_time = false;

bool adjust_feeding_time_1 = false;
bool adjust_X_feeding_time_1 = false;

bool adjust_feeding_time_2 = false;
bool adjust_X_feeding_time_2 = false;

bool is_peek_records = false;
bool clear_memory = false;

//button pin number
int btn_01 = 4;
int btn_02 = 5;
int btn_03 = 6;
int btn_04 = 7;
int btn_05 = 8;
int reset_button = 9;

int ServoPin1 = 10;
int ServoPin2 = 11;
int ServoPin3 = 12;
int relayPin = 13;
/*
relay -> 
ds3231 -> SDA, SCL
lcd ->  SDA, SCL, (A4, A5)
pin 3 -> RESET
IR sensor -> A0
ground -> servo circuit, main circuit
*/

int reset_count = 0;

int hourOFF = 0;
int minOFF =  0;
int secOFF = 0;

int hourON = 0;
int minON = 0;
int secON = 0;

int feedingHour = 0;
int feedingMin = 0;
int feedingSec = 0;

int x_feedHour = 0;
int x_feedMin = 0;
int x_feedSec = 0;

int feedingHour_1 = 0;
int feedingMin_1 = 0;
int feedingSec_1 = 0;

int x_feedHour_1 = 0;
int x_feedMin_1 = 0;
int x_feedSec_1 = 0;

int feedingHour_2 = 0;
int feedingMin_2 = 0;
int feedingSec_2 = 0;

int x_feedHour_2 = 0;
int x_feedMin_2 = 0;
int x_feedSec_2 = 0;

int blinkPos_OFF = 0;
int blinkPos_ON = 0;

int feedingPos = 0;
int x_feedingPos = 0;

int feedingPos_1 = 0;
int x_feedingPos_1 = 0;

int feedingPos_2 = 0;
int x_feedingPos_2 = 0;

int md_Val = 0;
int md = 0;

int num_Pos_OFF = 0;
int num_Pos_ON = 0;

int num_Pos_feeding= 0;
int num_Pos_Xfeed = 0;

int num_Pos_feeding_1 = 0;
int num_Pos_Xfeed_1 = 0;

int num_Pos_feeding_2 = 0;
int num_Pos_Xfeed_2 = 0;

int light_status = 0;

void setup() {
  
  lcd.begin();
  lcd.backlight();
	Serial.begin(9600);
	Serial.println("Serial Okay");
  
  pinMode(btn_01, INPUT);
  pinMode(btn_02, INPUT);
  pinMode(btn_03, INPUT);
  pinMode(btn_04, INPUT);
  pinMode(btn_05, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(reset_button, INPUT);

  myservo01.attach(ServoPin1);
  myservo02.attach(ServoPin2);
  myservo03.attach(ServoPin3);

	#ifdef ARDUINO_ARCH_ESP8266
		URTCLIB_WIRE.begin(0, 2); // D3 and D4 on ESP8266
	#else
		URTCLIB_WIRE.begin();
	#endif
	
  //rtc.set(30, 45, 17, 7, 16, 4, 23);
	//  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
  

  
  /*
  for (addr_EEPROM = 0; addr_EEPROM < 4096; addr_EEPROM++) { //EEPROM CLEAR DATA
      eeprom.eeprom_write(addr_EEPROM, (unsigned char) (0));
      if (int i = addr_EEPROM % 100 == 0) Serial.print("."); // Prints a '.' every 100 writes to EEPROM
    }
  */
   
  
  
  /*
  lcd.print("Checking Servos ");
  
for (int i = 0; i <= 5; i++){
  for (posTest = 0; posTest <= 180; posTest += 1) { 
      // in steps of 1 degree             
      myservo02.write(posTest);
      delay(5);                       
    }
      for (posTest = 180; posTest >= 0; posTest -= 1) { // goes from 180 degrees to 0 degrees              
        myservo02.write(posTest);
        delay(5);
      }
    }
  */
  eeprom.eeprom_read(50, &servoPos);
  myservo01.write(servoPos);
  myservo02.write(servoPos);
  myservo03.write(servoPos);
}

int count_digit(int nom){
    int count = 0;
    if(nom > 9999){
      count = 5;
    }else if(nom > 999){
      count = 4;
    }else if(nom > 99){
      count = 3;
    }else if(nom > 9){
      count = 2;
    }else{
      count = 1;
    }
    return count;
  }


void loop() {
  unsigned long currentTime = millis();

  int btn_right_check = digitalRead(btn_01);
  int btn_middle_check = digitalRead(btn_02);
  int btn_left_check = digitalRead(btn_03);

  int btn_inc_check = digitalRead(btn_04);
  int btn_dec_check = digitalRead(btn_05);

  int reset_value = digitalRead(reset_button);
  IR_sensor = digitalRead(A0);

    //memory flush user interaction (all button pressed 5x, then reset)
  if (currentTime - previousTime_2 >= eventTime_2_pushButtons){
    md_Val = digitalRead(btn_02);
    if (md_Val != 0){reset_count = 0;}
    if(reset_value != 0){reset_count++;}
    previousTime_2 = currentTime;
  }

  if(reset_count == 5){
    lcd.clear();
    lcd.print("Memory Reset");
    lcd.setCursor(0, 1);
    //EEPROM CLEAR DATA
    for (addr_EEPROM = 0; addr_EEPROM < 4096; addr_EEPROM++) { 
      currentTime = 0;
      eeprom.eeprom_write(addr_EEPROM, (unsigned char) (0));
      if (int i = addr_EEPROM % 100 == 0) lcd.print("."); // Prints a '.' every 100 writes to EEPROM
    }
    lcd.clear();
    lcd.print("RESET COMPLETE");
    reset_count = 0;
    pinMode(3, OUTPUT);
    return;
    }
  
  //DATA BACKUP IN CASE OF POWER SHORTAGE 
  
  if(rtc.lostPower() == true && IR_sensor != 0){ //update egg count 5 secs
      eeprom.eeprom_write(52, egg_count);
  }
  if(rtc.lostPower() == true){
    //save pos when turned off
     eeprom.eeprom_write(50, servoPos);
  }
  
  if(btn_right_check && btn_left_check != 0 && rtc.lostPower() == true || rtc.hour() == 23 && rtc.minute() == 59 && rtc.second() == 59){
    if (currentTime - previousTime_6 >= eventTime_6_write_EEPROM){ 
      eeprom.eeprom_write(0, hourOFF);
      eeprom.eeprom_write(2, minOFF);
      eeprom.eeprom_write(4, secOFF);
      eeprom.eeprom_write(6, hourON);
      eeprom.eeprom_write(8, minON);
      eeprom.eeprom_write(10, secON);
      eeprom.eeprom_write(12, feedingHour);
      eeprom.eeprom_write(14, feedingMin);
      eeprom.eeprom_write(16, feedingSec);
      eeprom.eeprom_write(18, x_feedHour);
      eeprom.eeprom_write(20, x_feedMin);
      eeprom.eeprom_write(22, x_feedSec);
      eeprom.eeprom_write(24, feedingHour_1);
      eeprom.eeprom_write(26, feedingMin_1);
      eeprom.eeprom_write(28, feedingSec_1);
      eeprom.eeprom_write(30, x_feedHour_1);
      eeprom.eeprom_write(32, x_feedMin_1);
      eeprom.eeprom_write(34, x_feedSec_1);
      eeprom.eeprom_write(36, feedingHour_2);
      eeprom.eeprom_write(38, feedingMin_2);
      eeprom.eeprom_write(40, feedingSec_2);
      eeprom.eeprom_write(42, x_feedHour_2);
      eeprom.eeprom_write(44, x_feedMin_2);
      eeprom.eeprom_write(46, x_feedSec_2);
      //48 light status
      //50 servo Pos
      //52-54 egg count
      //55 egg count recrod
      for(int i = 0; i <= day_count; i++){
        eeprom.eeprom_write(i + i + 55, egg_count_record_day[i]);
        Serial.print(egg_count_record_day[i]);
      };
      previousTime_6 = currentTime;
      Serial.println("Write Success");
    }
  }

  if (rtc.lostPower() == false){
    if (currentTime - previousTime_6 >= eventTime_6_write_EEPROM){ 
     eeprom.eeprom_read(0, &hourOFF);
     eeprom.eeprom_read(2, &minOFF);
     eeprom.eeprom_read(4, &secOFF);
     eeprom.eeprom_read(6, &hourON);
     eeprom.eeprom_read(8, &minON);
     eeprom.eeprom_read(10, &secON);
     eeprom.eeprom_read(12, &feedingHour);
     eeprom.eeprom_read(14, &feedingMin);
     eeprom.eeprom_read(16, &feedingSec);
     eeprom.eeprom_read(18, &x_feedHour);
     eeprom.eeprom_read(20, &x_feedMin);
     eeprom.eeprom_read(22, &x_feedSec);
     eeprom.eeprom_read(24, &feedingHour_1);
     eeprom.eeprom_read(26, &feedingMin_1);
     eeprom.eeprom_read(28, &feedingSec_1);
     eeprom.eeprom_read(30, &x_feedHour_1);
     eeprom.eeprom_read(32, &x_feedMin_1);
     eeprom.eeprom_read(34, &x_feedSec_1);
     eeprom.eeprom_read(36, &feedingHour_2);
     eeprom.eeprom_read(38, &feedingMin_2);
     eeprom.eeprom_read(40, &feedingSec_2);
     eeprom.eeprom_read(42, &x_feedHour_2);
     eeprom.eeprom_read(44, &x_feedMin_2);
     eeprom.eeprom_read(46, &x_feedSec_2);
     eeprom.eeprom_read(48, &light_status);
     eeprom.eeprom_read(50, &servoPos);
     eeprom.eeprom_read(52, &egg_count);
     for(int i = 0; i <= day_count; i++){
        eeprom.eeprom_read(i + i + 55, &egg_count_record_day[i]);
        Serial.print(egg_count_record_day[i]);
      };
     previousTime_6 = currentTime;
     Serial.println("Read Success");
    }
    egg_count_record_day[day_count] = egg_count;
    egg_count = egg_count - 1;
    rtc.lostPowerClear();
  }

  int hour_rtc = rtc.hour();
  int min_rtc = rtc.minute();
  int sec_rtc = rtc.second();
  rtc.refresh();

 
  //alarmOFF
  if (light_status == 0){
    digitalWrite(relayPin, LOW);
  }else if(light_status == 1){
    digitalWrite(relayPin, HIGH);
  }

  if(rtc.hour() == hourOFF && rtc.minute() == minOFF && rtc.second() == secOFF){
      //add save status
      if (currentTime - previousTime_6 >= eventTime_6_write_EEPROM){ 
        light_status = 0;
        eeprom.eeprom_write(48, light_status);
        Serial.println("LIGHTS OFF");
        previousTime_6 = currentTime;
      }
     }

  //alarmON
  if(rtc.hour() == hourON && rtc.minute() == minON && rtc.second() == secON){
     if (currentTime - previousTime_6 >= eventTime_6_write_EEPROM){
      light_status = 1;
      eeprom.eeprom_write(48, light_status);
      Serial.println("LIGHTS ON");
      previousTime_6 = currentTime;
       }
     }
  

  if (currentTime - previousTime_5 >= eventTime_5_moveServo){
    if(rtc.hour() == feedingHour && rtc.minute() == feedingMin && rtc.second() == feedingSec){
        Serial.println("FEED OPENED 3");
        servoPos = 180;
        myservo01.write(servoPos);
        for (int i = 0; i <= 5; i++){
          for (posTest = 0; posTest <= 180; posTest += 1) { 
              // in steps of 1 degree             
              myservo02.write(posTest);
              delay(5);                       
            }
              for (posTest = 180; posTest >= 0; posTest -= 1) { // goes from 180 degrees to 0 degrees              
                myservo02.write(posTest);
                delay(5);
            }
        }
        myservo03.write(servoPos);
      }
       if(rtc.hour() == feedingHour_1 && rtc.minute() == feedingMin_1 && rtc.second() == feedingSec_1){
        Serial.println("FEED OPENED 1");
        servoPos = 180;
        myservo01.write(servoPos);
        for (int i = 0; i <= 5; i++){
          for (posTest = 0; posTest <= 180; posTest += 1) { 
              // in steps of 1 degree             
              myservo02.write(posTest);
              delay(5);                       
            }
              for (posTest = 180; posTest >= 0; posTest -= 1) { // goes from 180 degrees to 0 degrees              
                myservo02.write(posTest);
                delay(5);
            }
        }
        myservo03.write(servoPos);
      }
       if(rtc.hour() == feedingHour_2 && rtc.minute() == feedingMin_2 && rtc.second() == feedingSec_2){
        Serial.println("FEED OPENED 2");
        servoPos = 180;
        myservo01.write(servoPos);
        for (int i = 0; i <= 5; i++){
          for (posTest = 0; posTest <= 180; posTest += 1) { 
              // in steps of 1 degree             
              myservo02.write(posTest);
              delay(5);                       
            }
              for (posTest = 180; posTest >= 0; posTest -= 1) { // goes from 180 degrees to 0 degrees              
                myservo02.write(posTest);
                delay(5);
            }
        }
        myservo03.write(servoPos);
      }

    //Close Feed
    if(rtc.hour() == x_feedHour && rtc.minute() == x_feedMin && rtc.second() == x_feedSec){
        Serial.println("FEED CLOSED 3");
        servoPos = 0;
        myservo01.write(servoPos);
        myservo02.write(servoPos);
        myservo03.write(servoPos);
      }
     
   if(rtc.hour() == x_feedHour_1 && rtc.minute() == x_feedMin_1 && rtc.second() == x_feedSec_1){
        Serial.println("FEED CLOSED 1");
        servoPos = 0;
        myservo01.write(servoPos);
        myservo02.write(servoPos);
        myservo03.write(servoPos);
      }
      
   if(rtc.hour() == x_feedHour_2 && rtc.minute() == x_feedMin_2 && rtc.second() == x_feedSec_2){
        Serial.println("FEED CLOSED 2");
        servoPos = 0;
        myservo01.write(servoPos);
        myservo02.write(servoPos);
        myservo03.write(servoPos);
       }
   previousTime_5 = currentTime;
  } 
  //counter
  if (currentTime - previousTime_3 >= eventTime_3_readIR){ //read every 1 ms
    if(state == 0){
      switch (IR_sensor) {
        case 1 : state = 1; egg_count++; break;
        case 0 : state = 0; break;
      }
    }
    if(IR_sensor == LOW){
      state = 0;
    }
    previousTime_3 = currentTime;
  }
   
  //if day ended reset count
  if (currentTime - previousTime_1 >= eventTime_1_updateLCD){
    if (rtc.hour() == 23 && rtc.minute() == 59 && rtc.second() == 58){
        //append to database
        egg_count_record_day[day_count] = egg_count;
        day_count++;
        egg_count = 0; // reset count for another day
        previousTime_1 = currentTime;
        Serial.print("data added");
      }
  }

  //if egg count database reached its, limit clear database
  if(day_count == 400){
    for (int i= 0; i < 400; i++) {
      egg_count_record_day[i] = 0;
      day_count = 0;
    }
    for (addr_EEPROM = 0; addr_EEPROM < 4096; addr_EEPROM++) { 
      eeprom.eeprom_write(addr_EEPROM, (unsigned char) (0));
      if (int i = addr_EEPROM % 100 == 0) lcd.print("."); // Prints a '.' every 100 writes to EEPROM
    }
    currentTime = 0;
    pinMode(2, OUTPUT);
    return;
  }
  
  //only active depending on mode count
  if (md_Val != 0){
    
     //restrict moving when adjusting is active
    if (is_peek_records || adjust_OFF || adjust_ON || adjust_feeding_time || adjust_X_feeding_time || adjust_feeding_time_1 || adjust_X_feeding_time_1 || adjust_feeding_time_2 || adjust_X_feeding_time_2 == true){
        return;
    }

    md++;
    md_Val = 0;
    lcd.noBlink();
    lcd.noCursor();

    if(md > 10){
      md = 0;
    }
  }else{    
    if (md == 0){ // display time
      //create another logic -> mode shortcut

      if (currentTime - previousTime_1 >= eventTime_1_updateLCD){
        lcd.clear();
        if (hour_rtc <= 12){
        //exception
        if (hour_rtc == 12){
          lcd.print("PM ");
          lcd.print(hour_rtc);
        }else{
          lcd.print("AM ");
          lcd.print(hour_rtc);
        }
      }
      if(hour_rtc >= 13){
          lcd.print("PM ");
          hour_rtc = hour_rtc - 12;
          lcd.print(hour_rtc);
        }
    
        lcd.print(':');
        lcd.print(min_rtc);
        lcd.print(':');
        lcd.print(sec_rtc);
        lcd.setCursor(0, 1);
        lcd.print(months[rtc.month() - 1]);
        lcd.print(" ");
        lcd.print(rtc.day());
        lcd.print(", ");
        lcd.print(rtc.year());
        lcd.print(" - ");
        lcd.print(days[rtc.dayOfWeek() - 1]);

        previousTime_1 = currentTime;
        }
      }
    else if (md == 1){ //Lights ON
      int btn_right = 0;
      int btn_left = 0;
      int btn_inc = 0;
      int btn_dec = 0;

      if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
        lcd.clear();
        lcd.print("Lights ON: ");
        lcd.setCursor(0, 1);
        lcd.print(hourON);
        lcd.print(":");
        lcd.print(minON);
        lcd.print(":");
        lcd.print(secON);
        lcd.setCursor(blinkPos_ON, 1);
        lcd.blink();
       
        btn_right = digitalRead(btn_01);
        btn_left = digitalRead(btn_03);
        btn_inc = digitalRead(btn_04);
        btn_dec = digitalRead(btn_05);

        previousTime_4 = currentTime;
      }

      int hourON_count = count_digit(hourON);
      int minON_count = count_digit(minON);
      int secON_count = count_digit(secON);
      
      num_Pos_ON = hourON_count + minON_count + secON_count + 2;

        if (adjust_ON == true){  
          if (btn_left && btn_right != 0){
            adjust_ON = false;
            return;
          }
          
          if (btn_left != 0){
            if(blinkPos_ON <= 0){
              blinkPos_ON = num_Pos_ON;
            }
            blinkPos_ON--;

          }else if (btn_right != 0){
            blinkPos_ON++;
            if(blinkPos_ON >= num_Pos_ON){
              blinkPos_ON = 0;
            }
          }
         
            if(btn_inc != 0){
            
              if(blinkPos_ON == 0){
                hourON++;
              }else if(hourON_count == 2 && blinkPos_ON == 1){
                hourON++;
              }

              if(blinkPos_ON == hourON_count + 1){
                minON++;
              }else if(minON_count == 2 && blinkPos_ON == minON_count + hourON_count){
                minON++;
              }

              if(blinkPos_ON == minON_count + hourON_count + 2){
                secON++;
              }else if(secON_count == 2 && blinkPos_ON == minON_count + hourON_count + secON_count + 1){
                secON++;
              }
              
            }else if(btn_dec != 0){
    
              if(blinkPos_ON == 0){
                hourON--;
              }else if(hourON_count == 2 && blinkPos_ON == 1){
                hourON--;
              }

              if(blinkPos_ON == hourON_count + 1){
                minON--;
              }else if(minON_count == 2 && blinkPos_ON == minON_count + hourON_count){
                minON--;
              }

              if(blinkPos_ON == minON_count + hourON_count + 2){
                secON--;
              }else if(secON_count == 2 && blinkPos_ON == minON_count + hourON_count + secON_count + 1){
                secON--;
              }
            }
          if(hourON >= 24){
              hourON = 0;
            }else if(minON >= 60){
              minON = 0;
            }else if(secON >= 60){
              secON = 0;
            }else if(minON < 0){
              minON = 59;
            }else if(secON < 0){
              secON = 59;
            }else if(hourON < 0){
              hourON = 23;
            }
          
        }else if (adjust_ON == false){
          if (btn_left && btn_right != 0){
            adjust_ON = true;
          }
          //exit
          lcd.noBlink();
        }
       
    }else if(md == 2){ //Lights OFF
      int btn_right = 0;
      int btn_left = 0;
      int btn_inc = 0;
      int btn_dec = 0;
      if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
        lcd.clear();
        lcd.print("Lights OFF: ");
        lcd.setCursor(0, 1);
        lcd.print(hourOFF);
        lcd.print(":");
        lcd.print(minOFF);
        lcd.print(":");
        lcd.print(secOFF);
        lcd.setCursor(blinkPos_OFF, 1);
        lcd.blink();
        btn_right = digitalRead(btn_01);
        btn_left = digitalRead(btn_03);
        btn_inc = digitalRead(btn_04);
        btn_dec = digitalRead(btn_05);
        previousTime_4 = currentTime;
      }
      
      int hourOFF_count = count_digit(hourOFF);
      int minOFF_count = count_digit(minOFF);
      int secOFF_count = count_digit(secOFF);
      num_Pos_OFF = hourOFF_count + minOFF_count + secOFF_count + 2;
      

      if (adjust_OFF == true){  
        if (btn_left && btn_right != 0){
          adjust_OFF = false;
          return;
        }
        
        if (btn_left != 0){
          if(blinkPos_OFF <= 0){
            blinkPos_OFF = num_Pos_OFF;
          }
          blinkPos_OFF--;
        }else if (btn_right != 0){
          blinkPos_OFF++;
          if(blinkPos_OFF >= num_Pos_OFF){
            blinkPos_OFF = 0;
          }
        }
          
          if(btn_inc != 0){
           
            if(blinkPos_OFF == 0){
              hourOFF++;
            }else if(hourOFF_count == 2 && blinkPos_OFF == 1){
              hourOFF++;
            }

            if(blinkPos_OFF == hourOFF_count + 1){
              minOFF++;
            }else if(minOFF_count == 2 && blinkPos_OFF == minOFF_count + hourOFF_count){
              minOFF++;
            }

            if(blinkPos_OFF == minOFF_count + hourOFF_count + 2){
              secOFF++;
            }else if(secOFF_count == 2 && blinkPos_OFF == minOFF_count + hourOFF_count + secOFF_count + 1){
              secOFF++;
            }
          }else if(btn_dec != 0){
  
            if(blinkPos_OFF == 0){
              hourOFF--;
            }else if(hourOFF_count == 2 && blinkPos_OFF == 1){
              hourOFF--;
            }

            if(blinkPos_OFF == hourOFF_count + 1){
              minOFF--;
            }else if(minOFF_count == 2 && blinkPos_OFF == minOFF_count + hourOFF_count){
              minOFF--;
            }

            if(blinkPos_OFF == minOFF_count + hourOFF_count + 2){
              secOFF--;
            }else if(secOFF_count == 2 && blinkPos_OFF == minOFF_count + hourOFF_count + secOFF_count + 1){
              secOFF--;
            }
          }
        }else if (adjust_OFF == false){
          if (btn_left && btn_right != 0){
            adjust_OFF = true;
          }
          //exit
          lcd.noBlink();
        }
        if(hourOFF >= 24){
            hourOFF = 0;
          }else if(minOFF >= 60){
            minOFF = 0;
          }else if(secOFF >= 60){
            secOFF = 0;
          }else if(minOFF < 0){
            minOFF = 59;
          }else if(secOFF < 0){
            secOFF = 59;
          }else if(hourOFF < 0){
            hourOFF = 23;
          }
    }else if (md == 3){ // open feed alarm 1

        int btn_right = 0;
        int btn_left = 0;
        int btn_inc = 0;
        int btn_dec = 0;

         if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
          Serial.print("Feeding 1");
          lcd.clear();
          lcd.print("Open Feed 1: ");
          lcd.setCursor(0, 1);
          lcd.print(feedingHour_1);
          lcd.print(":");
          lcd.print(feedingMin_1);
          lcd.print(":");
          lcd.print(feedingSec_1);
          btn_right = digitalRead(btn_01);
          btn_left = digitalRead(btn_03);
          btn_inc = digitalRead(btn_04);
          btn_dec = digitalRead(btn_05);
          lcd.setCursor(feedingPos_1, 1);
          lcd.blink();
          previousTime_4 = currentTime;
         }

        int feedingHour_count = count_digit(feedingHour_1);
        int feedingMin_count = count_digit(feedingMin_1);
        int feedingSec_count = count_digit(feedingSec_1);
        num_Pos_feeding_1 = feedingHour_count + feedingMin_count + feedingSec_count + 2;
        

        if (adjust_feeding_time_1 == true){  
          if (btn_left && btn_right != 0){
            adjust_feeding_time_1 = false;
            return;
          }
          
          if (btn_left != 0){
            if(feedingPos_1 <= 0){
              feedingPos_1 = num_Pos_feeding_1;
            }
            feedingPos_1--;
          }else if (btn_right != 0){
            feedingPos_1++;
            if(feedingPos_1 >= num_Pos_feeding_1){
              feedingPos_1 = 0;
            }
          }

            if(btn_inc != 0){
            
              if(feedingPos_1 == 0){
                feedingHour_1++;
              }else if(feedingHour_count == 2 && feedingPos_1 == 1){
                feedingHour_1++;
              }

              if(feedingPos_1 == feedingHour_count + 1){
                feedingMin_1++;
              }else if(feedingMin_count == 2 && feedingPos_1 == feedingMin_count + feedingHour_count){
                feedingMin_1++;
              }

              if(feedingPos_1 == feedingMin_count + feedingHour_count + 2){
                feedingSec_1++;
              }else if(feedingSec_count == 2 && feedingPos_1 == feedingMin_count + feedingHour_count + feedingSec_count + 1){
                feedingSec_1++;
              }
            }else if(btn_dec != 0){
    
              if(feedingPos_1 == 0){
                feedingHour_1--;
              }else if(feedingHour_count == 2 && feedingPos_1 == 1){
                feedingHour_1--;
              }

              if(feedingPos_1 == feedingHour_count + 1){
                feedingMin_1--;
              }else if(feedingMin_count == 2 && feedingPos_1 == feedingMin_count + feedingHour_count){
                feedingMin_1--;
              }

              if(feedingPos_1 == feedingMin_count + feedingHour_count + 2){
                feedingSec_1--;
              }else if(feedingSec_count == 2 && feedingPos_1 == feedingMin_count + feedingHour_count + feedingSec_count + 1){
                feedingSec_1--;
              }
            }
          }else if (adjust_feeding_time_1 == false){
            if (btn_left && btn_right != 0){
              adjust_feeding_time_1 = true;
            }
            //exit
            lcd.noBlink();
          }
         if(feedingHour_1 >= 24){
              feedingHour_1 = 0;
            }else if(feedingMin_1 >= 60){
              feedingMin_1 = 0;
            }else if(feedingSec_1 >= 60){
              feedingSec_1 = 0;
            }else if(feedingSec_1 < 0){
              feedingSec_1 = 59;
            }else if(feedingMin_1 < 0){
              feedingMin_1 = 59;
            }else if(feedingHour_1 < 0){
              feedingHour_1 = 23;
          }
    }else if (md == 4){ //  close feed alarm 1
      int btn_right = 0;
      int btn_left = 0;
      int btn_inc = 0;
      int btn_dec = 0;
      if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
       lcd.clear();
       lcd.print("Close Feed 1: ");
       lcd.setCursor(0, 1);
       lcd.print(x_feedHour_1);
       lcd.print(":");
       lcd.print(x_feedMin_1);
       lcd.print(":");
       lcd.print(x_feedSec_1);
       lcd.setCursor(x_feedingPos_1, 1);
       lcd.blink();
       btn_right = digitalRead(btn_01);
       btn_left = digitalRead(btn_03);
       btn_inc = digitalRead(btn_04);
       btn_dec = digitalRead(btn_05);
       previousTime_4 = currentTime;
      }
  
      int x_feedHour_count = count_digit(x_feedHour_1);
      int x_feedMin_count = count_digit(x_feedMin_1);
      int x_feedSec_count = count_digit(x_feedSec_1);
      num_Pos_Xfeed_1 = x_feedHour_count + x_feedMin_count + x_feedSec_count + 2;
    
      if (adjust_X_feeding_time_1 == true){  
        if (btn_left && btn_right != 0){
          adjust_X_feeding_time_1 = false;
          return;
        }
        
        if (btn_left != 0){
          if(x_feedingPos_1 <= 0){
            x_feedingPos_1 = num_Pos_Xfeed_1;
          }
          x_feedingPos_1--;
        }else if (btn_right != 0){
          x_feedingPos_1++;
          if(x_feedingPos_1 >= num_Pos_Xfeed_1){
            x_feedingPos_1 = 0;
          }
        }
      
          if(btn_inc != 0){
            
            if(x_feedingPos_1 == 0){
              x_feedHour_1++;
            }else if(x_feedHour_count == 2 && x_feedingPos_1 == 1){
              x_feedHour_1++;
            }

            if(x_feedingPos_1 == x_feedHour_count + 1){
              x_feedMin_1++;
            }else if(x_feedMin_count == 2 && x_feedingPos_1 == x_feedMin_count + x_feedHour_count){
              x_feedMin_1++;
            }
            if(x_feedingPos_1 == x_feedMin_count + x_feedHour_count + 2){
              x_feedSec_1++;
            }else if(x_feedSec_count == 2 && x_feedingPos_1 == x_feedMin_count + x_feedHour_count + x_feedSec_count + 1){
              x_feedSec_1++;
            }

          }else if(btn_dec != 0){
  
            if(x_feedingPos_1 == 0){
              x_feedHour_1--;
            }else if(x_feedHour_count == 2 && x_feedingPos_1 == 1){
              x_feedHour_1--;
            }

            if(x_feedingPos_1 == x_feedHour_count + 1){
              x_feedMin_1--;
            }else if(x_feedMin_count == 2 && x_feedingPos_1 == x_feedMin_count + x_feedHour_count){
              x_feedMin_1--;
            }

            if(x_feedingPos_1 == x_feedMin_count + x_feedHour_count + 2){
              x_feedSec_1--;
            }else if(x_feedSec_count == 2 && x_feedingPos_1 == x_feedMin_count + x_feedHour_count + x_feedSec_count + 1){
              x_feedSec_1--;
            }
          }
        }else if (adjust_X_feeding_time_1 == false){
          if (btn_left && btn_right != 0){
            adjust_X_feeding_time_1 = true;
          }
          //exit
          lcd.noBlink();
        }

        if(x_feedHour_1 >= 24){
            x_feedHour_1 = 0;
          }else if(x_feedMin_1 >= 60){
            x_feedMin_1 = 0;
          }else if(x_feedSec_1 >= 60){
            x_feedSec_1 = 0;
          }else if(x_feedSec_1 < 0){
            x_feedSec_1 = 59;
          }else if(x_feedMin_1 < 0){
            x_feedMin_1 = 59;
          }else if(x_feedHour_1 < 0){
            x_feedHour_1 = 23;
        } 
    }else if (md == 5){ //open feed alarm 2
      int btn_right = 0;
        int btn_left = 0;
        int btn_inc = 0;
        int btn_dec = 0;

         if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
          lcd.clear();
          lcd.print("Open Feed 2: ");
          lcd.setCursor(0, 1);
          lcd.print(feedingHour_2);
          lcd.print(":");
          lcd.print(feedingMin_2);
          lcd.print(":");
          lcd.print(feedingSec_2);
          btn_right = digitalRead(btn_01);
          btn_left = digitalRead(btn_03);
          btn_inc = digitalRead(btn_04);
          btn_dec = digitalRead(btn_05);
          lcd.setCursor(feedingPos_2, 1);
          lcd.blink();
          previousTime_4 = currentTime;
         }

        int feedingHour_count = count_digit(feedingHour_2);
        int feedingMin_count = count_digit(feedingMin_2);
        int feedingSec_count = count_digit(feedingSec_2);
        num_Pos_feeding_2 = feedingHour_count + feedingMin_count + feedingSec_count + 2;
        

        if (adjust_feeding_time_2 == true){  
          if (btn_left && btn_right != 0){
            adjust_feeding_time_2 = false;
            return;
          }
          
          if (btn_left != 0){
            if(feedingPos_2 <= 0){
              feedingPos_2 = num_Pos_feeding_2;
            }
            feedingPos_2--;
          }else if (btn_right != 0){
            feedingPos_2++;
            if(feedingPos_2 >= num_Pos_feeding_2){
              feedingPos_2 = 0;
            }
          }

            if(btn_inc != 0){
            
              if(feedingPos_2 == 0){
                feedingHour_2++;
              }else if(feedingHour_count == 2 && feedingPos_2 == 1){
                feedingHour_2++;
              }

              if(feedingPos_2 == feedingHour_count + 1){
                feedingMin_2++;
              }else if(feedingMin_count == 2 && feedingPos_2 == feedingMin_count + feedingHour_count){
                feedingMin_2++;
              }

              if(feedingPos_2 == feedingMin_count + feedingHour_count + 2){
                feedingSec_2++;
              }else if(feedingSec_count == 2 && feedingPos_2 == feedingMin_count + feedingHour_count + feedingSec_count + 1){
                feedingSec_2++;
              }
            }else if(btn_dec != 0){
    
              if(feedingPos_2 == 0){
                feedingHour_2--;
              }else if(feedingHour_count == 2 && feedingPos_2 == 1){
                feedingHour_2--;
              }

              if(feedingPos_2 == feedingHour_count + 1){
                feedingMin_2--;
              }else if(feedingMin_count == 2 && feedingPos_2 == feedingMin_count + feedingHour_count){
                feedingMin_2--;
              }

              if(feedingPos_2 == feedingMin_count + feedingHour_count + 2){
                feedingSec_2--;
              }else if(feedingSec_count == 2 && feedingPos_2 == feedingMin_count + feedingHour_count + feedingSec_count + 1){
                feedingSec_2--;
              }
            }
          }else if (adjust_feeding_time_2 == false){
            if (btn_left && btn_right != 0){
              adjust_feeding_time_2 = true;
            }
            //exit
            lcd.noBlink();
          }
         if(feedingHour_2 >= 24){
              feedingHour_2 = 0;
            }else if(feedingMin_2 >= 60){
              feedingMin_2 = 0;
            }else if(feedingSec_2 >= 60){
              feedingSec_2 = 0;
            }else if(feedingSec_2 < 0){
              feedingSec_2 = 59;
            }else if(feedingMin_2 < 0){
              feedingMin_2 = 59;
            }else if(feedingHour_2 < 0){
              feedingHour_2 = 23;
          }
    }
    else if (md == 6){ //close feed alarm 2
      int btn_right = 0;
      int btn_left = 0;
      int btn_inc = 0;
      int btn_dec = 0;
      if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
       lcd.clear();
       lcd.print("Close Feed 2: ");
       lcd.setCursor(0, 1);
       lcd.print(x_feedHour_2);
       lcd.print(":");
       lcd.print(x_feedMin_2);
       lcd.print(":");
       lcd.print(x_feedSec_2);
       lcd.setCursor(x_feedingPos_2, 1);
       lcd.blink();
       btn_right = digitalRead(btn_01);
       btn_left = digitalRead(btn_03);
       btn_inc = digitalRead(btn_04);
       btn_dec = digitalRead(btn_05);
       previousTime_4 = currentTime;
      }
  
      int x_feedHour_count = count_digit(x_feedHour_2);
      int x_feedMin_count = count_digit(x_feedMin_2);
      int x_feedSec_count = count_digit(x_feedSec_2);
      num_Pos_Xfeed_2 = x_feedHour_count + x_feedMin_count + x_feedSec_count + 2;
    
      if (adjust_X_feeding_time_2 == true){  
        if (btn_left && btn_right != 0){
          adjust_X_feeding_time_2 = false;
          return;
        }
        
        if (btn_left != 0){
          if(x_feedingPos_2 <= 0){
            x_feedingPos_2 = num_Pos_Xfeed_2;
          }
          x_feedingPos_2--;
        }else if (btn_right != 0){
          x_feedingPos_2++;
          if(x_feedingPos_2 >= num_Pos_Xfeed_2){
            x_feedingPos_2 = 0;
          }
        }
      
          if(btn_inc != 0){
            
            if(x_feedingPos_2 == 0){
              x_feedHour_2++;
            }else if(x_feedHour_count == 2 && x_feedingPos_2 == 1){
              x_feedHour_2++;
            }

            if(x_feedingPos_2 == x_feedHour_count + 1){
              x_feedMin_2++;
            }else if(x_feedMin_count == 2 && x_feedingPos_2 == x_feedMin_count + x_feedHour_count){
              x_feedMin_2++;
            }
            if(x_feedingPos_2 == x_feedMin_count + x_feedHour_count + 2){
              x_feedSec_2++;
            }else if(x_feedSec_count == 2 && x_feedingPos_2 == x_feedMin_count + x_feedHour_count + x_feedSec_count + 1){
              x_feedSec_2++;
            }

          }else if(btn_dec != 0){
  
            if(x_feedingPos_2 == 0){
              x_feedHour_2--;
            }else if(x_feedHour_count == 2 && x_feedingPos_2 == 1){
              x_feedHour_2--;
            }

            if(x_feedingPos_2 == x_feedHour_count + 1){
              x_feedMin_2--;
            }else if(x_feedMin_count == 2 && x_feedingPos_2 == x_feedMin_count + x_feedHour_count){
              x_feedMin_2--;
            }

            if(x_feedingPos_2 == x_feedMin_count + x_feedHour_count + 2){
              x_feedSec_2--;
            }else if(x_feedSec_count == 2 && x_feedingPos_2 == x_feedMin_count + x_feedHour_count + x_feedSec_count + 1){
              x_feedSec_2--;
            }
          }
        }else if (adjust_X_feeding_time_2 == false){
          if (btn_left && btn_right != 0){
            adjust_X_feeding_time_2 = true;
          }
          //exit
          lcd.noBlink();
        }

        if(x_feedHour_2 >= 24){
            x_feedHour_2 = 0;
          }else if(x_feedMin_2 >= 60){
            x_feedMin_2 = 0;
          }else if(x_feedSec_2 >= 60){
            x_feedSec_2 = 0;
          }else if(x_feedSec_2 < 0){
            x_feedSec_2 = 59;
          }else if(x_feedMin_2 < 0){
            x_feedMin_2 = 59;
          }else if(x_feedHour_2 < 0){
            x_feedHour_2 = 23;
        } 
  
    }else if(md == 7){ //Open Feed
        int btn_right = 0;
        int btn_left = 0;
        int btn_inc = 0;
        int btn_dec = 0;

         if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
          lcd.clear();
          lcd.print("Open Feed 3: ");
          lcd.setCursor(0, 1);
          lcd.print(feedingHour);
          lcd.print(":");
          lcd.print(feedingMin);
          lcd.print(":");
          lcd.print(feedingSec);
          btn_right = digitalRead(btn_01);
          btn_left = digitalRead(btn_03);
          btn_inc = digitalRead(btn_04);
          btn_dec = digitalRead(btn_05);
          lcd.setCursor(feedingPos, 1);
          lcd.blink();
          previousTime_4 = currentTime;
         }

        int feedingHour_count = count_digit(feedingHour);
        int feedingMin_count = count_digit(feedingMin);
        int feedingSec_count = count_digit(feedingSec);
        num_Pos_feeding = feedingHour_count + feedingMin_count + feedingSec_count + 2;
        

        if (adjust_feeding_time == true){  
          if (btn_left && btn_right != 0){
            adjust_feeding_time = false;
            return;
          }
          
          if (btn_left != 0){
            if(feedingPos <= 0){
              feedingPos = num_Pos_feeding;
            }
            feedingPos--;
          }else if (btn_right != 0){
            feedingPos++;
            if(feedingPos >= num_Pos_feeding){
              feedingPos = 0;
            }
          }

            if(btn_inc != 0){
            
              if(feedingPos == 0){
                feedingHour++;
              }else if(feedingHour_count == 2 && feedingPos == 1){
                feedingHour++;
              }

              if(feedingPos == feedingHour_count + 1){
                feedingMin++;
              }else if(feedingMin_count == 2 && feedingPos == feedingMin_count + feedingHour_count){
                feedingMin++;
              }

              if(feedingPos == feedingMin_count + feedingHour_count + 2){
                feedingSec++;
              }else if(feedingSec_count == 2 && feedingPos == feedingMin_count + feedingHour_count + feedingSec_count + 1){
                feedingSec++;
              }
            }else if(btn_dec != 0){
    
              if(feedingPos == 0){
                feedingHour--;
              }else if(feedingHour_count == 2 && feedingPos == 1){
                feedingHour--;
              }

              if(feedingPos == feedingHour_count + 1){
                feedingMin--;
              }else if(feedingMin_count == 2 && feedingPos == feedingMin_count + feedingHour_count){
                feedingMin--;
              }

              if(feedingPos == feedingMin_count + feedingHour_count + 2){
                feedingSec--;
              }else if(feedingSec_count == 2 && feedingPos == feedingMin_count + feedingHour_count + feedingSec_count + 1){
                feedingSec--;
              }
            }
          }else if (adjust_feeding_time == false){
            if (btn_left && btn_right != 0){
              adjust_feeding_time = true;
            }
            //exit
            lcd.noBlink();
          }
         if(feedingHour >= 24){
              feedingHour = 0;
            }else if(feedingMin >= 60){
              feedingMin = 0;
            }else if(feedingSec >= 60){
              feedingSec = 0;
            }else if(feedingSec < 0){
              feedingSec = 59;
            }else if(feedingMin < 0){
              feedingMin = 59;
            }else if(feedingHour < 0){
              feedingHour = 23;
          }
    }else if(md == 8){ //Close Feed
      int btn_right = 0;
      int btn_left = 0;
      int btn_inc = 0;
      int btn_dec = 0;
      if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
       lcd.clear();
       lcd.print("Close Feed 3: ");
       lcd.setCursor(0, 1);
       lcd.print(x_feedHour);
       lcd.print(":");
       lcd.print(x_feedMin);
       lcd.print(":");
       lcd.print(x_feedSec);
       lcd.setCursor(x_feedingPos, 1);
       lcd.blink();
       btn_right = digitalRead(btn_01);
       btn_left = digitalRead(btn_03);
       btn_inc = digitalRead(btn_04);
       btn_dec = digitalRead(btn_05);
       previousTime_4 = currentTime;
      }
  
      int x_feedHour_count = count_digit(x_feedHour);
      int x_feedMin_count = count_digit(x_feedMin);
      int x_feedSec_count = count_digit(x_feedSec);
      num_Pos_Xfeed = x_feedHour_count + x_feedMin_count + x_feedSec_count + 2;
    
      if (adjust_X_feeding_time == true){  
        if (btn_left && btn_right != 0){
          adjust_X_feeding_time = false;
          return;
        }
        
        if (btn_left != 0){
          if(x_feedingPos <= 0){
            x_feedingPos = num_Pos_Xfeed;
          }
          x_feedingPos--;
        }else if (btn_right != 0){
          x_feedingPos++;
          if(x_feedingPos >= num_Pos_Xfeed){
            x_feedingPos = 0;
          }
        }
      
          if(btn_inc != 0){
            
            if(x_feedingPos == 0){
              x_feedHour++;
            }else if(x_feedHour_count == 2 && x_feedingPos == 1){
              x_feedHour++;
            }

            if(x_feedingPos == x_feedHour_count + 1){
              x_feedMin++;
            }else if(x_feedMin_count == 2 && x_feedingPos == x_feedMin_count + x_feedHour_count){
              x_feedMin++;
            }

            if(x_feedingPos == x_feedMin_count + x_feedHour_count + 2){
              x_feedSec++;
            }else if(x_feedSec_count == 2 && x_feedingPos == x_feedMin_count + x_feedHour_count + x_feedSec_count + 1){
              x_feedSec++;
            }

          }else if(btn_dec != 0){
  
            if(x_feedingPos == 0){
              x_feedHour--;
            }else if(x_feedHour_count == 2 && x_feedingPos == 1){
              x_feedHour--;
            }

            if(x_feedingPos == x_feedHour_count + 1){
              x_feedMin--;
            }else if(x_feedMin_count == 2 && x_feedingPos == x_feedMin_count + x_feedHour_count){
              x_feedMin--;
            }

            if(x_feedingPos == x_feedMin_count + x_feedHour_count + 2){
              x_feedSec--;
            }else if(x_feedSec_count == 2 && x_feedingPos == x_feedMin_count + x_feedHour_count + x_feedSec_count + 1){
              x_feedSec--;
            }
          }
        }else if (adjust_X_feeding_time == false){
          if (btn_left && btn_right != 0){
            adjust_X_feeding_time = true;
          }
          //exit
          lcd.noBlink();
        }

        if(x_feedHour >= 24){
            x_feedHour = 0;
          }else if(x_feedMin >= 60){
            x_feedMin = 0;
          }else if(x_feedSec >= 60){
            x_feedSec = 0;
          }else if(x_feedSec < 0){
            x_feedSec = 59;
          }else if(x_feedMin < 0){
            x_feedMin = 59;
          }else if(x_feedHour < 0){
            x_feedHour = 23;
        } 
    }else if(md == 9){ //counter

      int btn_inc = 0;
      int btn_dec = 0;
      int btn_right = 0;
      int btn_left = 0;
      int day_count_num = count_digit(see_day);

      if (IR_sensor != 1 && is_peek_records == false){ //update lcd if IR is active
        lcd.clear();
        lcd.print("Day: ");
        lcd.print(day_count);
        lcd.setCursor(0, 1);
        lcd.print("Egg Count: ");
        lcd.print(egg_count);
      }

       if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
        btn_right = digitalRead(btn_01);
        btn_left = digitalRead(btn_03);
        btn_inc = digitalRead(btn_04);
        btn_dec = digitalRead(btn_05);
        if(is_peek_records == false){
          lcd.clear();
          lcd.print("Day: ");
          lcd.print(day_count);
          lcd.setCursor(0, 1);
          lcd.print("Egg Count: ");
          lcd.print(egg_count);
        }else if(is_peek_records == true){    
          if (btn_inc != 0 && is_peek_records == true){
            if(see_day >= day_count){
              return;
            }
            see_day++;
            // get array value 

          }else if(btn_dec != 0 && is_peek_records == true){
            if(see_day <= 0){
              return;
            }
            see_day--;

          }
          lcd.clear();
          lcd.print("Day: ");
          lcd.print(see_day);
          lcd.setCursor(0, 1);
          lcd.print("Egg Count: ");
          lcd.print(egg_count_record_day[see_day]);

          if(day_count_num == 2){
            lcd.setCursor(7, 0);
            lcd.print(" - ");
            lcd.print(day_count);
            lcd.setCursor(6, 0);
            lcd.blink();
          }else{
            lcd.setCursor(6, 0);
            lcd.print(" - ");
            lcd.print(day_count);
            lcd.setCursor(5, 0);
            lcd.blink();
          }
        }
        previousTime_4 = currentTime;
      }

      if (btn_right && btn_left != 0){
        if(is_peek_records == true){
          is_peek_records = false;
          
          lcd.noBlink();
          return;
        }
        see_day = day_count;
        is_peek_records = true;
      }
    }else if(md == 10){ //outside temp
      if (currentTime - previousTime_1 >= eventTime_1_updateLCD){
        lcd.clear();
        lcd.print("Outside Temp");
        lcd.setCursor(0, 1);
        lcd.print(rtc.temp() / 100);
        lcd.print(" C");
        previousTime_1 = currentTime;
      }      
    }
  }
}