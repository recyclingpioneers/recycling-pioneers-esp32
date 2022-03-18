/**
 * Recycling Pioneers
 * Adapted from various resources by Arneet Kalra
 * 
 * Libraries used for this sketch:
 * 
 * Ultrasonic sensor uses NewPing library
 * https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
 * 
 * HX711_ADC library to connect with load cell amplifier
 * 
 * Wifi Library for esp32
 */

//#include <WiFi.h>
#include <NewPing.h>
//#include <FirebaseESP32.h>
#include <HX711_ADC.h>
//#include "time.h"
//#include "arduino_secrets.h"

/*
 * The following code has been placed in a secret file for security reasons. The template follows as below:
#define SECRET_FIREBASE_HOST "firebase host here"
#define SECRET_FIREBASE_AUTH "firebase authentication here"
#define SECRET_WIFI_SSID "wifi name here"
#define SECRET_WIFI_PASSWORD "wifi password here"
*/

//Pins required for Distance Sensors 
#define TRIGGER_PIN_1  16  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN_1     17  // Arduino pin tied to echo pin on the ultrasonic sensor.

#define TRIGGER_PIN_2  18  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN_2     19  // Arduino pin tied to echo pin on the ultrasonic sensor.

#define TRIGGER_PIN_3  22  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN_3    23  // Arduino pin tied to echo pin on the ultrasonic sensor.

//Pins for Load Sensors
#define DOUT 5
#define CLK 4


//Information for time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; //Offset of 18000s (5 hours) for our timezone
const int   daylightOffset_sec = 3600; //Daylight saving offset one hour
/*
//Fields required for Firebase
FirebaseData firebaseData;
FirebaseJson json;
*/
//Fields required for weight
float calibrationValue = -134110.0; //Custom calibration value for our sensor
int weightData = 0.0000;
int last_stable_weight = -100.0000; //Set the inital stable to unattainable value
bool stable_weight_flag = false; //Set the weight to not stable yet
int ave;
int weight_history[4];
bool newDataReady = false;

//Fields required for distance
int distanceData = 0;
int last_stable_distance = -5; //Set the inital stable to unattainable value

//Field required for Time
char createdAt[30]; //Char array for time field
unsigned long previousMillis = 0; //Last time data was updated
#define MAX_DISTANCE 300 

HX711_ADC LoadCell(DOUT,CLK);   //HX711 constructor (dout pin, sck pin)

void setup() {
  Serial.begin(115200); // start serial port output, check for same speed at Serial Monitor
  initWeightSensor();
  Serial.println("Intialized");
}

void loop() {

      float weightData = LoadCell.update();
      weightData = fabs(LoadCell.getData());
      Serial.print("Load_cell output val: ");
      Serial.println(weightData);
}

void initWeightSensor(){
  // Set up the HX711 library
  Serial.println("Initializing Load Sensors...");
  LoadCell.begin();
  
  long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step

  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Load Sensors Startup & Tare: Online");
  }
}
