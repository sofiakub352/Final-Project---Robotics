#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "LiquidCrystal.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>   
#define SS_PIN 10
#define RST_PIN 9
SoftwareSerial mySerial(2, 3);
LiquidCrystal lcd(0);
Servo myServo;
MFRC522 mfrc522(SS_PIN, RST_PIN); 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
const int buzzerPin = 4;
const int firstButton = A3;
const int secondButton = A2; 
String myRfid="943be31e";  
String readRfid;
String IDname;  
int redLed = 7;
int greenLed = 6;
int method = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); 
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  myServo.write(0);
  myServo.attach(5);
  pinMode(firstButton, INPUT);
  pinMode(secondButton, INPUT);
  lcd.begin(20, 4);
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.print("Choose 1 or 2");
  finger.begin(57600);
  if (finger.verifyPassword()) {
   Serial.println("Found fingerprint sensor!");
  } 
  else {
   Serial.println("Did not find fingerprint sensor :(");
   while (1) { delay(1); }
  }
}

void cardOk() {  //function that prints on the lcd if the card was recognized
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Card recognized!!");
  lcd.setCursor(0, 1);
  lcd.print("Access granted!");
  lcd.setCursor(0, 2);
  lcd.print("Door will be");
  lcd.setCursor(0, 3);
  lcd.print("unlocked");
  delay(1000);
  lcd.clear();
  method = 3; 
}

void cardDenied() {  //function that prints on the lcd if the card was not recognized
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Card not recognized!!");
  lcd.setCursor(0, 1);
  lcd.print("Access denied!");
  delay(1000);
  lcd.clear();
  method = 3; 
}
void dump_byte_array(byte *buffer, byte bufferSize) {              //function that reads the RFID id
    readRfid="";
    for (byte i = 0; i < bufferSize; i++) {
        readRfid=readRfid + String(buffer[i], HEX);
    }
}

void open_lock() {                 //function that decides the movement of the Servo and keeps the door locked
  myServo.write(120); 
  delay(5000);
  myServo.write(0);
}

void getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  // found a match!
  Serial.print("Found ID #"); 
  Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); 
  Serial.println(finger.confidence);
  if(finger.fingerID == 1 || finger.fingerID == 2 || finger.fingerID == 3){
     IDname = "Sofia";
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Recognized Sofia");
     lcd.setCursor(0, 1);
     lcd.print("Access granted!");
     delay(1000);
     lcd.clear(); 
     open_lock();
     lcd.clear();
     method = 3;
  }
  else {
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Bad fingerprint");
     lcd.setCursor(0, 1);
     lcd.print("Access denied");
     delay(1000);
     lcd.clear();
     method = 3;
       }
}
int chooseMethod(int first, int second) {   //function that reads the input from the buttons and decides which method is selected
  int result;
  if(digitalRead(first) == HIGH)
     result = 1;
  else if(digitalRead(second) == HIGH)
     result = 2;
     if (result == 1 || result == 2) 
       return result;
     else 
       return 3;
}

void loop() {
 if(method != 3)
   method = chooseMethod(firstButton, secondButton); 
 switch(method) {
  case 1: //Card Method
    lcd.setCursor(0, 1);
    lcd.print("Scan your card");
    delay(1000);
    myServo.write(0); 
    if ( ! mfrc522.PICC_IsNewCardPresent())
          return;
    if ( ! mfrc522.PICC_ReadCardSerial())
          return;
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println("Hex code of card");
    Serial.println(readRfid);
    if (readRfid == myRfid ) { 
      digitalWrite(redLed, LOW);
      Serial.println("Access granted");
      digitalWrite(greenLed, HIGH);
      delay(2000);
      digitalWrite(greenLed, LOW);
      cardOk();
      open_lock(); 
      lcd.clear();
    }
    else if(readRfid != myRfid) {
      digitalWrite(redLed, HIGH);
      delay(2000);
      digitalWrite(redLed, LOW);
      cardDenied();
      lcd.clear();
    }
    break;
  case 2: //Fingerprint Method
    lcd.setCursor(0, 1);
    lcd.print("Scan your");
    lcd.setCursor(0, 2);
    lcd.print("fingerprint");
    myServo.write(0);
    getFingerprintIDez();  
    break;    
case 3: //return to the menu
    lcd.clear();
    lcd.print("Choose 1 or 2");
    method = chooseMethod(firstButton, secondButton); 
    break;
 }
}
