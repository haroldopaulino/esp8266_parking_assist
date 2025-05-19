#include <WiFi.h>
#include "FastLED.h"




const char* ssid     = "my_wifi_network";
const char* password = "my_wifi_password";
const char* baseUrl = "my_rest_url";




#define NUM_LEDS 150
#define DATA_PIN 12
// Clock pin only needed for SPI based chipsets when not using hardware SPI

CRGB leds[NUM_LEDS];

int trigPin1 = 15;    // Trigger
int echoPin1 = 2;    // Echo
int sensorInches1 = 0;

int trigPin2 = 0;    // Trigger
int echoPin2 = 4;    // Echo
int sensorInches2 = 0;

int trigPin3 = 16;    // Trigger
int echoPin3 = 17;    // Echo
int sensorInches3 = 0;

int trigPin4 = 5;    // Trigger
int echoPin4 = 18;    // Echo
int sensorInches4 = 0;

int whatUltrasonic = 0;

long duration, cm, inches;

int maximumDistanceInInches = 100;
int inactivityCounter = 0;
int inactivityTimeoutValue = 1500;
int sleepMode = 0;
int turnAllLightsOff = 0;
int toleranceValue = -1;

void setup() {
  Serial.begin (115200);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);
  
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  WiFi.begin(ssid, password);
}

void loop() {
  whatUltrasonic++;
  whatUltrasonic = (whatUltrasonic > 4 ? 1 : whatUltrasonic);

  readDistanceSensor(whatUltrasonic);
  //sleepMode = 0;
  delay(50);
  
  if ((sensorInches1 != 0) &&
      (sensorInches1 < sensorInches2 && sensorInches1 < sensorInches3 && sensorInches1 < sensorInches4)) {
    inches = sensorInches1;
    //turnAllWhite();
    Serial.print('Sensor 1: ');
  }

  if ((sensorInches2 != 0) &&
      (sensorInches2 < sensorInches1 && sensorInches2 < sensorInches3 && sensorInches2 < sensorInches4)) {
    inches = sensorInches2;
    //turnAllRed();
    Serial.print('Sensor 2: ');
  }

  if ((sensorInches3 != 0) &&
      (sensorInches3 < sensorInches1 && sensorInches3 < sensorInches2 && sensorInches3 < sensorInches4)) {
    inches = sensorInches3;
    //turnAllGreen();
    Serial.print('Sensor 3: ');
  }

  if ((sensorInches4 != 0) &&
      (sensorInches4 < sensorInches1 && sensorInches4 < sensorInches2 && sensorInches4 < sensorInches3)) {
    inches = sensorInches4;
    //turnAllBlue();
    sendLogDataToServer();
    Serial.print('Sensor 4: ');
  }
  
  Serial.println(inches);
  
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
