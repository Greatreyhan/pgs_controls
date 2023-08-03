#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Arduino.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// WiFi
const char *ssid = "infinergy";     // Enter your WiFi name
const char *password = "okeokeoke"; // Enter WiFi password

WiFiClient espClient;

/* 1. Define the WiFi credentials */
#define WIFI_SSID "infinergy"
#define WIFI_PASSWORD "okeokeoke"

/* 2. Define the API Key */
#define API_KEY "AIzaSyAj1WuRBgli6srEqC2Itd71H_xAptILN0o"

/* 3. Define the RTDB URL */
#define DATABASE_URL "pitrogreensystem-default-rtdb.firebaseio.com"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "pgsadmin@gmail.com"
#define USER_PASSWORD "Admin123"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

volatile int levelTanah = 0;
volatile int kelembapanTanah = 0;
volatile int intervalPompa = 0;
volatile int intervalPupuk = 0;
volatile int suhuUdara = 0;
volatile int kelembapanUdara = 0;
volatile int ppm = 0;
#define serialData D1
#define latchClock D2
#define shiftClock D3
#define PTA_PIN 0b00000001
#define PTB_PIN 0b00000010
#define TAP_PIN 0b00000100
#define TBP_PIN 0b00001000
#define PGH_PIN 0b00010000
#define PP_PIN  0b00100000
#define ROW_A   0b01000000
#define ROW_B   0b10000000
byte currentState = 0b00000000;
void turn_relay(byte binaryState, bool isOn);

volatile unsigned long previousMillis = 0;
volatile unsigned long interval = 5000;

void setup()
{
  pinMode(serialData, OUTPUT);
  pinMode(latchClock, OUTPUT);
  pinMode(shiftClock, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  turn_relay(0b11111111, false);
  delay(500);
}

void loop()
{
  unsigned long currentMillis = millis();
  if (Firebase.ready())
  {
    if (Firebase.getString(fbdo, "/controls/ALL"))
    {
      if (fbdo.dataType() == "string")
      {
        for(uint8_t i = 0; i < fbdo.stringData().length(); i++){
          char dataSwitch = fbdo.stringData().charAt(i);
          Serial.println(dataSwitch);
          if(dataSwitch == '0'){
            if(i == 0) turn_relay(PTA_PIN, false);
            else if(i == 1) turn_relay(PTB_PIN, false);
            else if(i == 2) turn_relay(TAP_PIN, false);
            else if(i == 3) turn_relay(TBP_PIN, false);
            else if(i == 4) turn_relay(PGH_PIN, false);
            else if(i == 5) turn_relay(PP_PIN, false);
          }
          if(dataSwitch == '1'){
            if(i == 0) turn_relay(PTA_PIN, true);
            else if(i == 1) turn_relay(PTB_PIN, true);
            else if(i == 2) turn_relay(TAP_PIN, true);
            else if(i == 3) turn_relay(TBP_PIN, true);
            else if(i == 4) turn_relay(PGH_PIN, true);
            else if(i == 5) turn_relay(PP_PIN, true);
          }
        }
      }
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.getString(fbdo, "/setting/ALL"))
    {
      if (fbdo.dataType() == "string")
      {
        for(uint8_t i = 0; i < fbdo.stringData().length(); i+=3){
          String paramData = fbdo.stringData().substring(i, i+3);
          Serial.println(paramData.toInt());
          if(i==0) levelTanah = paramData.toInt();
          else if(i==3) intervalPompa = paramData.toInt();
          else if(i==6) intervalPupuk = paramData.toInt();
          interval = intervalPompa;
        }
      }
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.getString(fbdo, "/param/ALL"))
    {
      if (fbdo.dataType() == "string")
      {
        for(uint8_t i = 0; i < fbdo.stringData().length(); i+=3){
          String paramData = fbdo.stringData().substring(i, i+3);
          Serial.println(paramData.toInt());
          if(i==0) suhuUdara = paramData.toInt();
          else if(i==3) kelembapanUdara = paramData.toInt();
          else if(i==6) kelembapanTanah = paramData.toInt();
          else if(i==9) ppm = paramData.toInt();
        }
      }
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }
  }
   if (currentMillis - previousMillis >= interval) {
    if(kelembapanTanah < levelTanah){
      turn_relay(PP_PIN, true);
    }
    else{
      turn_relay(PP_PIN, false);
    }
    previousMillis = currentMillis; 

  }
}

void turn_relay(byte binaryState, bool isOn){
  if(isOn){
    currentState |= binaryState;
    digitalWrite(latchClock, LOW);
    shiftOut(serialData, shiftClock, MSBFIRST, currentState);
    digitalWrite(latchClock, HIGH);
    Serial.println(currentState);
  }
  else{
    currentState &= (~binaryState);
    digitalWrite(latchClock, LOW);
    shiftOut(serialData, shiftClock, MSBFIRST, currentState);
    digitalWrite(latchClock, HIGH);
    Serial.println(currentState);
  }
}
