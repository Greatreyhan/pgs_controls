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
#define PTA_PIN D0
#define PTB_PIN D1
#define TAP_PIN D2
#define TBP_PIN D4
#define PGH_PIN D5
#define PP_PIN  D6
void setup()
{
  pinMode(PTA_PIN, OUTPUT);
  pinMode(PTB_PIN, OUTPUT);
  pinMode(TAP_PIN, OUTPUT);
  pinMode(TBP_PIN, OUTPUT);
  pinMode(PGH_PIN, OUTPUT);
  pinMode(PP_PIN, OUTPUT);
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
}

void loop()
{
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
            if(i == 0) digitalWrite(PTA_PIN, LOW);
            else if(i == 1) digitalWrite(PTB_PIN, LOW);
            else if(i == 2) digitalWrite(TAP_PIN, LOW);
            else if(i == 3) digitalWrite(TBP_PIN, LOW);
            else if(i == 4) digitalWrite(PGH_PIN, LOW);
            else if(i == 5) digitalWrite(PP_PIN, LOW);
          }
          if(dataSwitch == '1'){
            if(i == 0) digitalWrite(PTA_PIN, HIGH);
            else if(i == 1) digitalWrite(PTB_PIN, HIGH);
            else if(i == 2) digitalWrite(TAP_PIN, HIGH);
            else if(i == 3) digitalWrite(TBP_PIN, HIGH);
            else if(i == 4) digitalWrite(PGH_PIN, HIGH);
            else if(i == 5) digitalWrite(PP_PIN, HIGH);
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
}
