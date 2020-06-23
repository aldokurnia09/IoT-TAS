#include "FirebaseESP8266.h"
#include <SPI.h> // SPI library
#include <MFRC522.h> // RFID library (https://github.com/miguelbalboa/rfid)
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h> // 1602A library (https://github.com/johnrickman/LiquidCrystal_I2C)

#define FIREBASE_HOST "https://tasiot-71160003.firebaseio.com/"
#define FIREBASE_AUTH "FVREOlRTsT3xtWPVaXNFojS1uZFf6x5hk4k5GuvB"

constexpr uint8_t RST_PIN = 2;
constexpr uint8_t SS_PIN = 0;
String path = "/Node1";
bool rfidTapped = false;
const char* ssid = "SSID";
const char* password = "PASSWORD";
int accessValue = 0;
int lockValue = 0;
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
LiquidCrystal_I2C lcd(0x27,16,2);
FirebaseData firebaseData;

void setup() {
  // put your setup code here, to run once:
  delay(3000);
  Serial.begin(9600);   // Initiate a serial communication

  WiFi.begin(ssid, password);
  Serial.print(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Connecting to Firebase
  Serial.println(F("\nConnected"));
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); //Connect to Firebase
  Firebase.reconnectWiFi(true);

  // RFID
  SPI.begin();      // Initiate SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details

  // 1602A LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Aldo / 71160003");
  Firebase.getInt(firebaseData, path + "/access"); //get sound value from firebase
  accessValue = firebaseData.intData();
  lcd.setCursor(0,1);
  if(accessValue == 0){
    lcd.print(F("NO ACCESS"));
    rfidTapped = false;
    Serial.println("Control Not Active");
  }
  else{
    lcd.print(F("ACCESS"));
    rfidTapped = true;
    Serial.println("Control Active");
  }
  Serial.println("------------------------------------\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (mfrc522.PICC_IsNewCardPresent()) { // Look for new cards
    if (mfrc522.PICC_ReadCardSerial()) { // Select one of the cards
      Serial.println("ID Card Get");
      Serial.print("Card ID : ");

      String content = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        // Dump a byte array as hex values to Serial
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);

        // Dump a byte array as hex values to String
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX)); //
      }

      content.toUpperCase(); // Make string become UPPERCASE
      mfrc522.PICC_HaltA();
      if (content.substring(1) == "B7 78 5E 62") { // Change UID of the card that you want to give access, for example A0 B1 C2 D3
        // Do something with access granted
        Serial.println();
        Serial.println("---Access Granted---");

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Aldo / 71160003");
        if(rfidTapped == false){
          rfidTapped = true;
          lcd.setCursor(0,1);
          lcd.print(F("ACCESS"));
          Serial.println("Control Active");
          Firebase.setInt(firebaseData, path + "/access", 1);
          accessValue = 1;
          Serial.println();
        } 
        else if(rfidTapped == true){
          rfidTapped = false;
          lcd.setCursor(0,1);
          lcd.print(F("NO ACCESS"));
          Serial.println("Control Not Active");
          Firebase.setInt(firebaseData, path + "/access", 0);
          accessValue = 0;
          Serial.println();
        }
      }
      else {
        // Do something with access denied
        Serial.println();
        Serial.println("---Access Denied---");
      }  
    }
  }

  Firebase.getInt(firebaseData, path + "/lock"); //get sound value from firebase
  int locked = firebaseData.intData();
  if(accessValue == 1){
    if(locked == 0){
      lcd.setCursor(6,1);
      lcd.print(F("-LOCK  "));
    }
    else{
      lcd.setCursor(6,1);
      lcd.print(F("-UNLOCK"));
    }
  }
}
