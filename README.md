# A ESP8266 based Parking Assist that helps park a car in a garage, as close to the wall as 1 inch. The main functions include:

#### 1) Connect to the WiFi network using the ESP8266 WiFi card;
#### 2) Control a 2811 LED strip using the FastLED library;
#### 3) Calculate the distance from the car to the wall by using a ultrasonic sensor;
#### 4) Based on the distance, translate that into progressively lighting the LED strip with the following pattern:
   * Green color for longer distances over 50 inches
   * Yellow color for distances between 30 and 50 inches
   * Red color for distances less than 30 inches
#### 5) Send a log to the rest service when the lights go to sleep;
