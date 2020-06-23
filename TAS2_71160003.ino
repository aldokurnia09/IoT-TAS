#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Stepper.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";

#define FIREBASE_HOST "https://tasiot-71160003.firebaseio.com/"
#define FIREBASE_AUTH "FVREOlRTsT3xtWPVaXNFojS1uZFf6x5hk4k5GuvB"

const int stepsPerRevolution = 200;
String path = "/Node1";
int temp = 0;
Stepper myStepper(stepsPerRevolution, 5, 4, 14, 12); // IN1, IN3, IN2, IN4
FirebaseData firebaseData;

void setup() {
  delay(3000);
  Serial.begin(9600);
  
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

  Firebase.getInt(firebaseData, path + "/lock"); //get sound value from firebase
  temp = firebaseData.intData();
  
  // set the speed at 80 rpm:
  myStepper.setSpeed(80);
}

void loop() {
  Firebase.getInt(firebaseData, path + "/lock"); //get sound value from firebase
  int locked = firebaseData.intData();
  if(temp!=locked){
    temp = locked;
    if(locked == 0){
      myStepper.step(-200);
      myStepper.step(-200);
    }
    else{
      myStepper.step(200);
      myStepper.step(200);
    }
  }
}
