#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// #include <iostream>
// #include <string> // for string
// using namespace std;

// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"

// Network credentials
#define WIFI_SSID "xxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxx"

// Firebase project API Key
#define API_KEY "xxxxxxxx"

// Firebase Realtime database URL
#define DATABASE_URL "xxxxxxxx"

// Define Firebase data object
FirebaseData fbdo;

// Define Firebase authentication object
FirebaseAuth auth;

// Define Firebase configuration object
FirebaseConfig config;

unsigned long dataMillis = 0;

unsigned long sendDataPrevMillis = 0;

bool signupOK = false;

// define pin connections for the ultrasonic sensor
// trig pin receives control sig from arduino
const int trigPin = 5;
// echo pin sends a sig to arduino which then
// measures the duration of pulse to calculate distance
const int echoPin = 6;
// define pin connection for led
const int ledPin = 4;
// define the distance threshold
const int distanceThreshold = 14;
// define the variables to store the distance and duration of the ultrasonic pulse
float duration;
int distance;
int percentage;

void setup()
{
  // Initialize serial and wait for port to open:
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
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

  // assign API Key
  config.api_key = API_KEY;

  // assign RTDB URL
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // config.signer.test_mode = true;

  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.reconnectWiFi(true);

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // set the trig pin as output and echo pin as input
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // set the led pin as an output
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  // generate 10-microsecond pulse to trig pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // measure duration(us) of pulse from echo pin
  duration = pulseIn(echoPin, HIGH);
  // calculate the distance(cm)
  distance = 0.017 * duration;

  if (distance < distanceThreshold)
  {
    // led is lit
    digitalWrite(ledPin, HIGH);
  }
  else
  {
    // led is off
    digitalWrite(ledPin, LOW);
  }

  // print the value to Serial Monitor
  Serial.print("distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  percentage = (distance / 26) * 100;
  String percentage_str = String(percentage);

  if (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)
  {
    dataMillis = millis();
    Serial.printf("Set dist... %s\n", Firebase.RTDB.setInt(&fbdo, "/Boxes/a/Status", distance) ? "ok" : fbdo.errorReason().c_str());
  }

  delay(500);
}