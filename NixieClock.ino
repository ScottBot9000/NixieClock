// Nixie-Bot
// Created 2013
// Scott-Bot.com

#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h> 

const int button1Pin = 11;
const int button2Pin = 10;

int led = 13;

RTC_DS1307 RTC;
DateTime now;

//Pin connected to ST_CP of 74HC595
int latchPin = 6;
//Pin connected to SH_CP of 74HC595
int clockPin = 7;
////Pin connected to DS of 74HC595
int dataPin = 5;

const int debouncetime = 100; // In Milliseconds

int button1State = 0;
int button1_debounced = 0;
int button1_debounced_prev = 0;
int button1_debounced_re = 0;
unsigned long button1count = 0; // Button 1 Counter

int button2State = 0;
int button2_debounced = 0;
int button2_debounced_prev = 0;
int button2_debounced_re = 0;
unsigned long button2count = 0; // Button 2 Counter

int settime_en = 0;
int settime_digit_count = 0;
int settimedigits[] = {0, 0, 0, 0, 0, 0};
int maxdigits[] = {2, 9, 5, 9, 5, 9};

char settimechararray[9] = {'0', '0', ':', '0', '0', ':', '0', '0', '\0'};   //declaring character array
String settimestr;  //declaring string
char tempchar[1] = {'0'};

int ledState = 0;

unsigned long SecondCount = 0; // Second counter
unsigned long MinuteCount = 0; // Second counter
int randint;

byte DataOut1 = B11111111;
byte DataOut2 = B11111111;
byte DataOut3 = B11111111;

void setup () {
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);       
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(led, OUTPUT);     
  Serial.begin(57600);
  Wire.begin();
  RTC.begin();
  
  
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
    
  Serial.print("Programmed: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
    
  //Wait for power supply to power up
  delay(2000);

}

void loop () {

  // Every minute display a test pattern
  if (millis()-MinuteCount > 60000) {
    MinuteCount = millis();
    ScrollPattern();
  } else if (millis()-SecondCount > 1000) {
    SecondCount = millis();
    GetTime();
    DispTime();
    
    // Blink the LED
    if (ledState == HIGH) {
      digitalWrite(led, HIGH);
      ledState = LOW;
    } else {
      digitalWrite(led, LOW);
      ledState = HIGH;
    }
  }
  
  // Debounce the buttons
  Debounce();
  
  // Button functions
  if (button1_debounced_re == HIGH) {   
    // Display the scrolling pattern
    ScrollPattern();
    MinuteCount = millis();
  } 
  
  if (button2_debounced_re == HIGH) {   
    // Display the scrolling pattern
    SetTime();
    MinuteCount = millis();
  } 
  
  
}

void SetTime() {
  
  settime_en = HIGH;
  settime_digit_count = 1;
  settimedigits[1] = 0;
  settimedigits[2] = 0;
  settimedigits[3] = 0;
  settimedigits[4] = 0;
  settimedigits[5] = 0;
  settimedigits[6] = 0;
  
  // Blank the Digits
  SetDigit(1, 0);
  for (int i = 2; i<=6; i++) {
    SetDigit(i, 10);
  }
  shiftOut();
  
  while(settime_en == HIGH){
    // Debounce the buttons
    Debounce();
    
    if (button2_debounced_re == HIGH) {
      
      settime_digit_count = settime_digit_count + 1;
      if (settime_digit_count == 7) {
        settime_en = LOW;
      }
      
      SetDigit(settime_digit_count, settimedigits[settime_digit_count]);
      shiftOut();
    }
    
    if (button1_debounced_re == HIGH) {
      if (settimedigits[settime_digit_count] == maxdigits[settime_digit_count-1]) {
        settimedigits[settime_digit_count] = 0;
      } else {
        settimedigits[settime_digit_count] = settimedigits[settime_digit_count] + 1;
      }
      
      SetDigit(settime_digit_count, settimedigits[settime_digit_count]);
      shiftOut();
    }
    
  }
  
      Serial.print("Final Time: ");
      Serial.print(settimedigits[1]);
      Serial.print(settimedigits[2]);
      Serial.print(settimedigits[3]);
      Serial.print(settimedigits[4]);
      Serial.print(settimedigits[5]);
      Serial.println(settimedigits[6]);
      
      // Set the time
      
      String str1 = "";
      String timestr = str1 + settimedigits[1] + settimedigits[2] + ":" + settimedigits[3] + settimedigits[4] + ":" + settimedigits[5] + settimedigits[6]; 
      
      Serial.print("Final String: ");
      Serial.println(timestr);
      
      char timechararray[9];
      timestr.toCharArray(timechararray, 9); 
      
      // Send the time to the RTC
      Serial.println("Adjusting RTC Time!");
      Serial.println(timechararray);
      RTC.adjust(DateTime(__DATE__, timechararray));
      
}


// Displays the time
void DispTime() {
  int hourdigit1 =(now.hour())/10;
  int hourdigit2 =(now.hour())%10;
  int mindigit1 =(now.minute())/10;
  int mindigit2 =(now.minute())%10;
  int secdigit1 =(now.second())/10;
  int secdigit2 =(now.second())%10;
  SetDigit(1, hourdigit1);
  SetDigit(2, hourdigit2);
  SetDigit(3, mindigit1);
  SetDigit(4, mindigit2);
  SetDigit(5, secdigit1);
  SetDigit(6, secdigit2);
  shiftOut();
}

void ScrollPattern() {
  // Blank the digits
  SetDigit(1, 10);
  SetDigit(2, 10);
  SetDigit(3, 10);
  SetDigit(4, 10);
  SetDigit(5, 10);
  SetDigit(6, 10);
  delay(250);
  //Scroll Loop
  for (int i = 1; i<=6; i++) {
    SetDigit(1, i);
    SetDigit(2, i-1);
    SetDigit(3, i-2);
    SetDigit(4, i-3);
    SetDigit(5, i-4);
    SetDigit(6, i-5);
    shiftOut();
    delay(200);
  }
  for (int i = 7; i<=39; i++) {
    SetDigit(1, (i)%10);
    SetDigit(2, (i-1)%10);
    SetDigit(3, (i-2)%10);
    SetDigit(4, (i-3)%10);
    SetDigit(5, (i-4)%10);
    SetDigit(6, (i-5)%10);
    shiftOut();
    delay(200);
  }
  for (int i = 10; i<=16; i++) {
    SetDigit(1, i);
    SetDigit(2, i-1);
    SetDigit(3, i-2);
    SetDigit(4, i-3);
    SetDigit(5, i-4);
    SetDigit(6, i-5);
    shiftOut();
    delay(200);
  }
}

// Sets a digit of the display
void SetDigit(int Digit, int Number) {
  
  byte BinNum;
  
  if ( Number == 0) {	
    BinNum= B0000;
  }
  else if ( Number == 1) {	
    BinNum= B1001;
  }
  else if ( Number == 2) {	
    BinNum= B1000;
  }
  else if ( Number == 3) {	
    BinNum= B0111;
  }
  else if ( Number == 4) {	
    BinNum= B0110;
  }
  else if ( Number == 5) {	
    BinNum= B0101;
  }
  else if ( Number == 6) {	
    BinNum= B0100;
  }
  else if ( Number == 7) {	
    BinNum= B0011;
  }
  else if ( Number == 8) {	
    BinNum= B0010;
  }
  else if ( Number == 9) {	
    BinNum= B0001;
  }
  else {
    BinNum= B1010;
  }
  
  if ( Digit == 1) {	
    DataOut1 &= B00001111;
    DataOut1 |= (BinNum << 4);
  }
  else if ( Digit == 2) {	
    DataOut1 &= B11110000;
    DataOut1 |= BinNum;
  }
  else if ( Digit == 3) {	
    DataOut2 &= B00001111;
    DataOut2 |= (BinNum << 4);
  }
  else if ( Digit == 4) {	
    DataOut2 &= B11110000;
    DataOut2 |= BinNum;
  }
  else if ( Digit == 5) {	
    DataOut3 &= B00001111;
    DataOut3 |= (BinNum << 4);
  }
  else if ( Digit == 6) {	
    DataOut3 &= B11110000;
    DataOut3 |= BinNum;
  }
  
}  

// Shifts out the data to the shift registers
void shiftOut() {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;

  //clear everything out just in case to
  //prepare shift register for bit shifting
  //ground latchPin and hold low for as long as you are transmitting
  digitalWrite(latchPin, 0);
  digitalWrite(dataPin, 0);
  digitalWrite(clockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(clockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( DataOut1 & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(dataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(clockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(dataPin, 0);
  }
  
  //stop shifting
  digitalWrite(clockPin, 0);
  
  // Shift the second word
  for (i=7; i>=0; i--)  {
    digitalWrite(clockPin, 0);

    if ( DataOut2 & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }
    
    digitalWrite(dataPin, pinState);
    digitalWrite(clockPin, 1);
    digitalWrite(dataPin, 0);
  }

  digitalWrite(clockPin, 0);
  
  // Shift the third word
  for (i=7; i>=0; i--)  {
    digitalWrite(clockPin, 0);

    if ( DataOut3 & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }

    digitalWrite(dataPin, pinState);
    digitalWrite(clockPin, 1);
    digitalWrite(dataPin, 0);
  }

  digitalWrite(clockPin, 0);
  
  digitalWrite(latchPin, 1);
}

// Read from the real time clock
void GetTime () {
  now = RTC.now();
}

// Debounce buttons
void Debounce () {
  button1State = digitalRead(button1Pin);
  button2State = digitalRead(button2Pin);
  
  if (button1State == HIGH) {
    if (button1count == debouncetime) {
      button1count = debouncetime;
      button1_debounced = HIGH;
    } else {
      button1count = button1count + 1;
    }
  } else {
    button1count = LOW;
    button1_debounced = LOW;
  }
  
  if (button2State == HIGH){
    if (button2count == debouncetime) {
      button2count = debouncetime;
      button2_debounced = HIGH;
    } else {
      button2count = button2count + 1;
    }
  } else {
    button2count = LOW;
    button2_debounced = LOW;
  }
  
  if ((button1_debounced_prev == LOW) && (button1_debounced == HIGH)) {
    button1_debounced_re = HIGH;
    Serial.println("Button 1 press!");
  } else {
    button1_debounced_re = LOW;
  }
  
  if ((button2_debounced_prev == LOW) && (button2_debounced == HIGH)) {
    button2_debounced_re = HIGH;
    Serial.println("Button 2 press!");
  } else {
    button2_debounced_re = LOW;
  } 
  
  button1_debounced_prev = button1_debounced;
  button2_debounced_prev = button2_debounced;
}
