#include <WiFi.h>
#include "FastLED.h"

const char* ssid     = "my_wifi_network";
const char* password = "my_wifi_password";
const char* baseUrl = "my_rest_url";

#define NUM_LEDS 150
#define DATA_PIN 6
// Clock pin only needed for SPI based chipsets when not using hardware SPI

CRGB leds[NUM_LEDS];

int trigPin = 12;    // Trigger
int echoPin = 11;    // Echo
long duration, cm, inches;

int inactivityCounter = 0;
int inactivityTimeoutValue = 150;
int sleepMode = 0;
int turnAllLightsOff = 0;
int toleranceValue = -1;

void setup() {
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  WiFi.begin(ssid, password);
}

void loop() {
  readDistanceSensor();
  //sleepMode = 0;
  delay(200);
  /*inches = 5;
  processGreen();
    processYellow();
    processRed();
    FastLED.show();*/
    
  if (sleepMode == 0) {
    resetAllLights();
    processGreen();
    processYellow();
    processRed();
    
    FastLED.show();
  } else {
    if (turnAllLightsOff == 0) {
      turnAllLightsOff = 1;
      turnAllOff();
    }
  }
  Serial.println();
  inactivityCheck();
}

void readDistanceSensor() {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
  //Serial.print("inches: ");
  //Serial.print(inches);
}

void resetAllLights() {
  if (inches > 10) {
    turnAllOff();
  } else {
    if (sleepMode == 0) {
      sendLogDataToServer();
      turnAllWhite();
    } else {
      turnAllOff();
    }
  }
}

void turnAllOff() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0,0,0);
  }
}

void turnAllWhite() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255,255,255);
  }
}

void processGreen() {
  int greenInches = 0;
  int activeGreenLight = 0;
  if (inches <= 50) activeGreenLight = 1;
  if (inches > 30 && inches <= 50) greenInches = 50 - inches;
  if (inches <= 30) greenInches = 20;
  
  if (activeGreenLight == 1) {
    //Serial.print("  -  greenInches: ");
    //Serial.print(greenInches);
    for(int i = 0; i <= greenInches; i++) {
      leds[i] = CRGB(0,255,0);
      leds[99 - i] = CRGB(0,255,0);
      leds[i + 100] = CRGB(0,255,0);
    }
  }
}

void processYellow() {
  int yellowInches = 0;
  int activeYellowLight = 0;
  if (inches <= 30) activeYellowLight = 1;
  if (inches > 10 && inches <= 30) yellowInches = 30 - inches;
  if (inches <= 10) yellowInches = 20;

  if (activeYellowLight == 1) {
    //Serial.print("  -  yellowInches: ");
    //Serial.print(yellowInches);
    for(int i = 0; i <= yellowInches; i++) {
      leds[i + 20] = CRGB(255,255,0);
      leds[79 - i] = CRGB(255,255,0);
      leds[i + 120] = CRGB(255,255,0);
    }
  }
}

void processRed() {
  int redInches = 0;
  if (inches <= 10) redInches = 10 - inches;

  if (redInches > 0) {
    //Serial.print("redInches: ");
    //Serial.print(redInches);
    for(int i = 0; i <= redInches; i++) {
      leds[i + 40] = CRGB(255,0,0);
      leds[59 - i] = CRGB(255,0,0);
      leds[140 + i] = CRGB(255,0,0);
    }
  }
}

void inactivityCheck() {
  if (inches == (toleranceValue - 1) ||
      inches == toleranceValue ||
      inches == (toleranceValue + 1)) {
    if (inactivityCounter < inactivityTimeoutValue) {
      inactivityCounter++;
      sleepMode = 0;
      turnAllLightsOff = 0;
    }
    if (inactivityCounter == inactivityTimeoutValue) {
      turnAllLightsOff = 1;
      if (sleepMode == 0) {
        turnAllOff();
        FastLED.show();
      }
      sleepMode = 1;
    }
  } else {
    toleranceValue = inches;
    inactivityCounter = 0;
    sleepMode = 0;
    turnAllLightsOff = 0;
  }
}

void sendLogDataToServer() {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(baseUrl, httpPort)) {
      Serial.println("connection failed");
      return;
  }

  // We now create a URI for the request
  String url = "/web/parkd/gateway.php";
  url += "?sensorInches1=";
  url += sensorInches1;
  url += "&sensorInches2=";
  url += sensorInches2;
  url += "&sensorInches3=";
  url += sensorInches3;
  url += "&sensorInches4=";
  url += sensorInches4;

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + baseUrl + "\r\n" +
               "Connection: close\r\n\r\n");
}

void readDistanceSensor(int targetSensor) {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  int inputTriggerPin;
  int inputEchoPin;
  switch (targetSensor) {
    case 1:
      inputTriggerPin = trigPin1;
      inputEchoPin = echoPin1;
      break;
    case 2:
      inputTriggerPin = trigPin2;
      inputEchoPin = echoPin2;
      break;
    case 3:
      inputTriggerPin = trigPin3;
      inputEchoPin = echoPin3;
      break;
    case 4:
      inputTriggerPin = trigPin4;
      inputEchoPin = echoPin4;
      break;
    default:
      inputTriggerPin = trigPin1;
      inputEchoPin = echoPin1;
      break;
  }
  digitalWrite(inputTriggerPin, LOW);
  delayMicroseconds(5);
  digitalWrite(inputTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(inputTriggerPin, LOW);
 
  pinMode(inputEchoPin, INPUT);
  duration = pulseIn(inputEchoPin, HIGH);
 
  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135

  switch (targetSensor) {
    case 1:
      sensorInches1 = inches;
      break;
    case 2:
      sensorInches2 = inches;
      break;
    case 3:
      sensorInches3 = inches;
      break;
    case 4:
      sensorInches4 = inches;
      break;
    default:
      sensorInches1 = inches;
      break;
  }
  //Serial.print("inches: ");
  //Serial.print(inches);
}

void resetAllLights() {
  if (inches > 10) {
    turnAllOff();
  } else {
    if (sleepMode == 0) {
      turnAllWhite();
    } else {
      turnAllOff();
    }
  }
}

void turnAllOff() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0,0,0);
  }
}

void turnAllWhite() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255,255,255);
  }
}

void processStatus() {
  
}

void inactivityCheck() {
  if (inches == (toleranceValue - 1) ||
      inches == toleranceValue ||
      inches == (toleranceValue + 1)) {
    if (inactivityCounter < inactivityTimeoutValue) {
      inactivityCounter++;
      sleepMode = 0;
      turnAllLightsOff = 0;
    }
    if (inactivityCounter == inactivityTimeoutValue) {
      turnAllLightsOff = 1;
      if (sleepMode == 0) {
        turnAllOff();
        FastLED.show();
      }
      sleepMode = 1;
    }
  } else {
    toleranceValue = inches;
    inactivityCounter = 0;
    sleepMode = 0;
    turnAllLightsOff = 0;
  }
}

