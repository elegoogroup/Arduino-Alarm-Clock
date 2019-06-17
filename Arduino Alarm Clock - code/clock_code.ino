//Libraries
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <dht_nonblocking.h>




//Connections and constants 
#define DHT_SENSOR_TYPE DHT_TYPE_11
LiquidCrystal lcd(2,3,4,5,6,7);
int tempC;
double tempK;
int tempReading;
unsigned long starttime;
float temperature;
float humidity;
byte clocksymbol[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b01110,
  0b11111,
  0b00000,
  0b00100,
  0b00000,
};

byte temp[8] = {
  0b10000,
  0b00111,
  0b01000,
  0b01000,
  0b01000,

  
  0b01000,
  0b00111,
  0b00000,
}; 

static const int DHT_SENSOR_PIN = 10;
const int buzzer = 9;
const int isbacklight = 8;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );






RTC_DS1307 rtc; //DS1307 i2c
char daysOfTheWeek[7][12] = {"Sunday","Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int btSet = A5;
const int btAdj = A4;
const int btAlarm = A3;

const int shakeSensor = A3;
long interval = 300;  
int melody[] = { 600, 800, 1000,1200 };

//Variables
int DD,MM,YY,H,M,S,set_state, adjust_state, alarm_state,AH,AM, shake_state;
int shakeTimes=0;
int i =0;
int btnCount = 0;
String sDD;
String sMM;
String sYY;
String sH;
String sM;
String sS;
String aH="12";
String aM="00";
String alarm = "     ";
long previousMillis = 0;    

//Boolean flags
boolean setupScreen = false;
boolean alarmON=false;
boolean turnItOn = false;
boolean backlightON=true;
   
void setup() {
  //Init RTC and LCD library items
  rtc.begin();
  lcd.begin(16,2);
  //Set outputs/inputs
  pinMode(btSet,INPUT_PULLUP);
  pinMode(btAdj,INPUT_PULLUP);
  pinMode(btAlarm, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);
  lcd.createChar(1, clocksymbol);
  lcd.createChar(2, temp);

  digitalWrite(isbacklight,HIGH);
  
  
  //Check if RTC has a valid time/date, if not set it to 00:00:00 01/01/2018.
  //This will run only at first time or if the coin battery is low.
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // This line sets the RTC with an explicit date & time, for example to set
    // January 1, 2018 at 00:00am you would call:
    rtc.adjust(DateTime(2018, 01, 01, 00, 00, 0)); 
  }
  delay(100);
  //Read alarm time from EEPROM memmory
  AH=EEPROM.read(0);
  AM=EEPROM.read(1);
  //Check if the numbers that you read are valid. (Hours:0-23 and Minutes: 0-59)
  if (AH>23){
    AH=0;
  }
  if (AM>59){
    AM=0;
  }


//starttime=millis();

 
  
}

void loop() {
  readBtns();       //Read buttons 
  getTimeDate();    //Read time and date from RTC
  if (!setupScreen){
    lcdPrint();     //Normanlly print the current time/date/alarm to the LCD
    if (alarmON){
      callAlarm();   // and check the alarm if set on
    }
    else{
         lcd.setCursor(10, 0);
         lcd.write(" ");
        }
  }
  else{
    timeSetup();    //If button set is pressed then call the time setup function
     
  }

   if( measure_environment( &temperature, &humidity ) == true )
  {
     tempC =temperature;  
  } 

  
}



static bool measure_environment( float *temperature, float *humidity )
 {
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
  if( millis( ) - measurement_timestamp > 3000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }
  return( false );
}









/*************** Functions ****************/
//Read buttons state
void readBtns(){
  set_state = digitalRead(btSet);
  adjust_state = digitalRead(btAdj);
  alarm_state = digitalRead(btAlarm);
  if(!setupScreen){
    if (alarm_state==LOW){
      if (alarmON){
        alarm="     ";
        alarmON=false;
      }
      else{
        alarmON=true;
      }
      delay(500);
    }

  if (adjust_state==LOW){
      if (backlightON==true){
        backlightON=false;
        digitalWrite(isbacklight,LOW);
      }
      else{
        backlightON=true;
        digitalWrite(isbacklight,HIGH);
      }
      delay(500);
    }

    
  }
  if (set_state==LOW){
    if(btnCount<7){
      btnCount++;
      setupScreen = true;
        if(btnCount==1){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("------SET------");
          lcd.setCursor(0,1);
          lcd.print("-TIME and DATE-");
          delay(2000);
          lcd.clear();
        }
    } 
    else{
      lcd.clear();
      rtc.adjust(DateTime(YY, MM, DD, H, M, 0)); //Save time and date to RTC IC
      EEPROM.write(0, AH);  //Save the alarm hours to EEPROM 0
      EEPROM.write(1, AM);  //Save the alarm minuted to EEPROM 1
      lcd.print("Saving....");
      delay(2000);
      lcd.clear();
      setupScreen = false;
      btnCount=0;
    }
    delay(500);
  }
}

//Read time and date from rtc ic
void getTimeDate(){
  if (!setupScreen){
    DateTime now = rtc.now();
    DD = now.day();
    MM = now.month();
    YY = now.year();
    H = now.hour();
    M = now.minute();
    S = now.second();
  }
  //Make some fixes...
  if (DD<10){ sDD = '0' + String(DD); } else { sDD = DD; }
  if (MM<10){ sMM = '0' + String(MM); } else { sMM = MM; }
  sYY=YY-2000;
  if (H<10){ sH = '0' + String(H); } else { sH = H; }
  if (M<10){ sM = '0' + String(M); } else { sM = M; }
  if (S<10){ sS = '0' + String(S); } else { sS = S; }
  if (AH<10){ aH = '0' + String(AH); } else { aH = AH; }
  if (AM<10){ aM = '0' + String(AM); }  else { aM = AM; }
}
//Print values to the display
void lcdPrint(){
  String line1 = sH+":"+sM+":"+sS+" |";
  lcd.setCursor(0,0); //First row
   lcd.print(line1);
   if (alarmON){
     lcd.setCursor(10, 0);
     lcd.write(1);
               }
  String line2 = aH+":"+aM;
   lcd.setCursor(11, 0);
   lcd.print(line2);
        
  String line3 = sDD+"-"+sMM+"-"+sYY +" | " +tempC ;
  lcd.setCursor(0,1); //Second row
  lcd.print(line3);  
  if (setupScreen){
    lcd.setCursor(13, 1);
    lcd.print("");  
  }
  else{
    lcd.setCursor(13, 1);
    lcd.write(2);
  }
  
}

//Setup screen
void timeSetup(){
  int up_state = adjust_state;
  int down_state = alarm_state;
  if(btnCount<=5){
    if (btnCount==1){         //Set Hour
      lcd.setCursor(4,0);
      lcd.print(">"); 
      if (up_state == LOW){   //Up button +
        if (H<23){
          H++;
        }
        else {
          H=0;
        }
        delay(350);
      }
      if (down_state == LOW){ //Down button -
        if (H>0){
          H--;
        }
        else {
          H=23;
        }
        delay(350);
      }
    }
    else if (btnCount==2){      //Set  Minutes
      lcd.setCursor(4,0);
      lcd.print(" ");
      lcd.setCursor(9,0);
      lcd.print(">");
      if (up_state == LOW){
        if (M<59){
          M++;
        }
        else {
          M=0;
        }
        delay(350);
      }
      if (down_state == LOW){
        if (M>0){
          M--;
        }
        else {
          M=59;
        }
        delay(350);
      }
    }
    else if (btnCount==3){      //Set Day
      lcd.setCursor(9,0);
      lcd.print(" ");
      lcd.setCursor(0,1);
      lcd.print(">");
      if (up_state == LOW){
        if (DD<31){
          DD++;
        }
        else {
          DD=1;
        }
        delay(350);
      }
      if (down_state == LOW){
        if (DD>1){
          DD--;
        }
        else {
          DD=31;
        }
        delay(350);
      }
    }
    else if (btnCount==4){      //Set Month
      lcd.setCursor(0,1);
      lcd.print(" ");
      lcd.setCursor(5,1);
      lcd.print(">");
      if (up_state == LOW){
        if (MM<12){
          MM++;
        }
        else {
          MM=1;
        }
        delay(350);
      }
      if (down_state == LOW){
        if (MM>1){
          MM--;
        }
        else {
          MM=12;
        }
        delay(350);
      }
    }
    else if (btnCount==5){      //Set Year
      lcd.setCursor(5,1);
      lcd.print(" ");
      lcd.setCursor(10,1);
      lcd.print(">");
      if (up_state == LOW){
        if (YY<2999){
          YY++;
        }
        else {
          YY=2000;
        }
        delay(350);
      }
      if (down_state == LOW){
        if (YY>2018){
          YY--;
        }
        else {
          YY=2999;
        }
        delay(350);
      }
    }
    lcd.setCursor(5,0);
    lcd.print(sH);
    lcd.setCursor(8,0);
    lcd.print(":");
    lcd.setCursor(10,0);
    lcd.print(sM);
    lcd.setCursor(1,1);
    lcd.print(sDD);
    lcd.setCursor(4,1);
    lcd.print("-");
    lcd.setCursor(6,1);
    lcd.print(sMM);
    lcd.setCursor(9,1);
    lcd.print("-");
    lcd.setCursor(11,1);
    lcd.print(sYY);
  }
  else{
    setAlarmTime();
  }
}

//Set alarm time
void setAlarmTime(){
  int up_state = adjust_state;
  int down_state = alarm_state;
  String line2;
  lcd.setCursor(0,0);
  lcd.print("SET  ALARM TIME");
  if (btnCount==6){             //Set alarm Hour
    if (up_state == LOW){
      if (AH<23){
        AH++;
      }
      else {
        AH=0;
      }
      delay(350);
    }
    if (down_state == LOW){
      if (AH>0){
        AH--;
      }
      else {
        AH=23;
      }
      delay(350);
    }
    line2 = "    >"+aH+" : "+aM+"    ";
  }
  else if (btnCount==7){ //Set alarm Minutes
    if (up_state == LOW){
      if (AM<59){
        AM++;
      }
      else {
        AM=0;
      }
      delay(350);
    }
    if (down_state == LOW){
      if (AM>0){
        AM--;
      }
      else {
        AM=59;
      }
      delay(350);
    }
    line2 = "     "+aH+" :>"+aM+"    ";    
  }
  lcd.setCursor(0,1);
  lcd.print(line2);
}

void callAlarm(){
  if (aM==sM && aH==sH && S>=0 && S<=10){
    turnItOn = true;
  } 
 else if(alarm_state==LOW ||(S>=59)){    //||(M==(AM+1))
    turnItOn = false;
    alarmON=true;
    delay(50);
  } 
  /*if(analogRead(shakeSensor)>1000){
    shakeTimes++;
    Serial.print(shakeTimes);
    delay(50);
  } */
  if (turnItOn){
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;   
      tone(buzzer,melody[i],100);
      i++; 
      if(i>3){i=0; };
      Serial.println("00000");
    }
  }
  else{
    noTone(buzzer);
   
  }
}
