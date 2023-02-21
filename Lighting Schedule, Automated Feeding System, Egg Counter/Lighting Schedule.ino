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
bool is_peek_records = false;
bool clear_memory = false;

//button pin number
int reset_button = 4;
int btn_01 = 5;
int btn_02 = 6;
int btn_03 = 7;
int btn_04 = 8;
int btn_05 = 9;
int ServoPin1 = 10;
int ServoPin2 = 11;

/*
relay -> 
ds3231 -> SDA, SCL
lcd ->  SDA, SCL, (A4, A5)
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

int blinkPos_OFF = 0;
int blinkPos_ON = 0;
int feedingPos = 0;
int x_feedingPos = 0;

int md_Val = 0;
int md = 0;

int num_Pos_OFF = 0;
int num_Pos_ON = 0;
int num_Pos_feeding= 0;
int num_Pos_Xfeed = 0;

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
  pinMode(reset_button, INPUT);

  myservo01.attach(ServoPin1);
  myservo02.attach(ServoPin2);

	#ifdef ARDUINO_ARCH_ESP8266
		URTCLIB_WIRE.begin(0, 2); // D3 and D4 on ESP8266
	#else
		URTCLIB_WIRE.begin();
	#endif

	//rtc.set(0, 5, 18, 2, 21, 2, 23);
	//  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
  

  
  /*
  for (addr_EEPROM = 0; addr_EEPROM < 4096; addr_EEPROM++) { //EEPROM CLEAR DATA
      eeprom.eeprom_write(addr_EEPROM, (unsigned char) (0));
      if (int i = addr_EEPROM % 100 == 0) Serial.print("."); // Prints a '.' every 100 writes to EEPROM
    }
  */
   
  
  /*
  lcd.print("Checking Servos ");
  
  
  for (posTest = 0; posTest <= 180; posTest += 1) { 
      // in steps of 1 degree
      myservo01.write(posTest);              
      myservo02.write(posTest);
      delay(15);                       
    }
    for (posTest = 180; posTest >= 0; posTest -= 1) { // goes from 180 degrees to 0 degrees
      myservo01.write(posTest);              
      myservo02.write(posTest);
      delay(15);                       
    }
  */
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

  int btn_inc_check = digitalRead(btn_04);
  int btn_dec_check = digitalRead(btn_05);
  int btn_right_check = digitalRead(btn_01);
  int btn_left_check = digitalRead(btn_03);
  int btn_middle_check = digitalRead(btn_02);
  int reset_value = digitalRead(reset_button);
  IR_sensor = digitalRead(A0);

    //memory flush user interaction (all button pressed 5x, then reset)
  if (currentTime - previousTime_2 >= eventTime_2_pushButtons){
    md_Val = digitalRead(btn_02);
    if (md_Val != 0){reset_count = 0;}
    if(reset_value != 0){
    reset_count++;
    }
    previousTime_2 = currentTime;
  }

  if(reset_count == 5){
    lcd.clear();
    lcd.print("Memory Reset");
    lcd.setCursor(0, 1);
    //EEPROM CLEAR DATA
    for (addr_EEPROM = 0; addr_EEPROM < 4096; addr_EEPROM++) { 
      eeprom.eeprom_write(addr_EEPROM, (unsigned char) (0));
      if (int i = addr_EEPROM % 100 == 0) lcd.print("."); // Prints a '.' every 100 writes to EEPROM
    }
    lcd.clear();
    lcd.print("RESET COMPLETE");
    reset_count = 0;
    return;
    }
  
 
  
  //DATA BACKUP IN CASE OF POWER SHORTAGE 
  
  if(rtc.lostPower() == true && IR_sensor != 0){ //update egg count 5 secs
      eeprom.eeprom_write(26, egg_count);
  }
  
  
  if(btn_inc_check || btn_dec_check != 0 && rtc.lostPower() == true){
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
      eeprom.eeprom_write(24, day_count);
      for(int i = 0; i <= day_count; i++){
        eeprom.eeprom_write(i + i + 28, egg_count_record_day[i]);
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
     eeprom.eeprom_read(24, &day_count);
     eeprom.eeprom_read(26, &egg_count);
     for(int i = 0; i <= day_count; i++){
        eeprom.eeprom_read(i + i + 28, &egg_count_record_day[i]);
        Serial.print(egg_count_record_day[i]);
      };
     previousTime_6 = currentTime;
     Serial.println("Read Success");
    }
  }

  int hour_rtc = rtc.hour();
  int min_rtc = rtc.minute();
  int sec_rtc = rtc.second();
  rtc.refresh();

  //alarmOFF
  if(rtc.hour() == hourOFF && rtc.minute() == minOFF && rtc.second() == secOFF){
      Serial.println("LIGHTS OFF");
     }

  //alarmON
  if(rtc.hour() == hourON && rtc.minute() == minON && rtc.second() == secON){
      Serial.println("LIGHTS ON");
     }

   if (currentTime - previousTime_5 >= eventTime_5_moveServo){
      //Open Feed
    if(rtc.hour() == feedingHour && rtc.minute() == feedingMin && rtc.second() == feedingSec){
        Serial.println("FEED OPENED");
        servoPos = 180;
        myservo01.write(servoPos);
        myservo02.write(servoPos);
      }

    //Close Feed
    if(rtc.hour() == x_feedHour && rtc.minute() == x_feedMin && rtc.second() == x_feedSec){
        Serial.println("FEED CLOSED");
        servoPos = 0;
        myservo01.write(servoPos);
        myservo02.write(servoPos);
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
    if (rtc.hour() == 23 && rtc.minute() == 59 && rtc.second() == 59){
        //append to database
        egg_count_record_day[day_count] = egg_count;
        day_count++;
        egg_count = 0; // reset count for another day
        previousTime_1 = currentTime;
      }
  }

  //if egg count database reached its, limit clear database
  if(day_count == 365){
    for (int i= 0; i < 365; i++) {
      egg_count_record_day[i] = 0;
      day_count = 0;
    }
  }
  
  //only active depending on mode count
  if (md_Val != 0){
    
     //restrict moving when adjusting is active
    if (is_peek_records || adjust_OFF || adjust_ON || adjust_feeding_time || adjust_X_feeding_time == true){
        return;
    }

    md++;
    md_Val = 0;
    lcd.noBlink();
    lcd.noCursor();

    if(md > 5){
      md = 0;
    }
  }else{    
    if (md == 0){ // display time
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
      

    else if (md == 1){ //Lighting Schedule
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
       
    }else if(md == 2){ //Lighting Schedule
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
    }else if(md == 3){ //Open Feed
        int btn_right = 0;
        int btn_left = 0;
        int btn_inc = 0;
        int btn_dec = 0;

         if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
          lcd.clear();
          lcd.print("Open Feed: ");
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
    }else if(md == 4){ //Close Feed
      int btn_right = 0;
      int btn_left = 0;
      int btn_inc = 0;
      int btn_dec = 0;
      if (currentTime - previousTime_4 >= eventTime_4_updateLCD2){
       lcd.clear();
       lcd.print("Close Feed: ");
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
    }else if(md == 5){ //counter

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
    }
  }
}




