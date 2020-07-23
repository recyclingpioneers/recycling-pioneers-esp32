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
 * 
 * Time Library 
 */

#include <WiFi.h>
#include <NewPing.h>
#include <FirebaseESP32.h>
#include <HX711_ADC.h>
#include "time.h"
#include "arduino_secrets.h"

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

#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define UPDATE_INTERVAL 60000 //Time in milliseconds between updating firebase database (1 min)


const char *FIREBASE_HOST = SECRET_FIREBASE_HOST;
const char *FIREBASE_AUTH = SECRET_FIREBASE_AUTH;
const char *WIFI_SSID = SECRET_WIFI_SSID;
const char *WIFI_PASSWORD = SECRET_WIFI_PASSWORD;


//Information for time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; //Offset of 18000s (5 hours) for our timezone
const int   daylightOffset_sec = 3600; //Daylight saving offset one hour

//Fields required for Firebase
FirebaseData firebaseData;
FirebaseJson json;

//Fields required for weight
float calibrationValue = 20764.00; //Custom calibration value for our sensor
float weightData = 0.00;
float ave;
float weight_history[4];

//Fields required for distance
int distanceData = 0;

//Field required for Time
char createdAt[30]; //Char array for time field
unsigned long previousMillis = 0; //Last time updated


NewPing sonar1(TRIGGER_PIN_1, ECHO_PIN_1, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar2(TRIGGER_PIN_2, ECHO_PIN_2, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar3(TRIGGER_PIN_3, ECHO_PIN_3, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
HX711_ADC LoadCell(DOUT,CLK);   //HX711 constructor (dout pin, sck pin)

void connectToFirebaseDatabase(){
  //Connect to firebase database
  Serial.println("Beginning Connection with Firebase Realtime Database...");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Serial.println("Connected to Firebase Database: Welcome");
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

int getDistanceData(){
  int distance1 = sonar1.convert_cm(sonar1.ping_median(10)); // read distance from ultrasonic sensor (using median of 10 iterations)
  int distance2 = sonar2.convert_cm(sonar2.ping_median(10)); // read distance from ultrasonic sensor (using median of 10 iterations)
  int distance3 = sonar3.convert_cm(sonar3.ping_median(10)); // read distance from ultrasonic sensor (using median of 10 iterations)
  int finalDistance = (distance1 + distance2 + distance3)/3;
  return finalDistance; 
}

boolean weightISR() {
  if (LoadCell.update()) {
    weightData = LoadCell.getData();
    
    //Get the last 4 weight measurements and compare them 
    weight_history[3] = weight_history[2];
    weight_history[2] = weight_history[1];
    weight_history[1] = weight_history[0];
    weight_history[0] = weightData;
    ave = (weight_history[0] + weight_history[1] + weight_history[2] + weight_history[3])/4;

    if ((abs(ave - weightData) < 0.10) && weightData >= 0.00) {
        return true;
    }
    else {
      return false;
    }
  }
}

String getLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return ("Failed to obtain time");
  }

  strftime(createdAt,50, "%c", &timeinfo);  
  Serial.println(createdAt);

  return (createdAt);
}

void sendToFirebase(String t){

  json.set("Distance",distanceData);
  json.set("Weight", weightData);
  json.set("Time", t);

  if (Firebase.pushJSON(firebaseData, "Sensors/Sensor1/Data", json)) {
    //SUCCESS
  } else {
    Serial.print("Error in sending JSON data to firebase, ");
    Serial.println(firebaseData.errorReason());
  }
 
  //*********************
  //Update Latest Data
   if (Firebase.setInt(firebaseData, "Sensors/Sensor1/LatestDistance", distanceData)) {
    //SUCCESS
   } else {
    //Failed?, get the error reason from firebaseData
    Serial.print("Error in sending latest distance data, ");
    Serial.println(firebaseData.errorReason());
  }
  
  if (Firebase.setFloat(firebaseData, "Sensors/Sensor1/LatestWeight", weightData)) {
    //Success
  } else {
    //Failed?, get the error reason from firebaseData
    Serial.print("Error in sending latest weight data, ");
    Serial.println(firebaseData.errorReason());
  } 
}

void setup() {
  Serial.begin(115200); // start serial port output, check for same speed at Serial Monitor
  
   //Connect to wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("Succesfully Connected to the Internet.");
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalTime();
  connectToFirebaseDatabase(); //Once online, connect to the database

  //Configure the weight sensor
  initWeightSensor();
}

void loop() {
  unsigned long currentMillis = millis();
  LoadCell.update();
  //Only run every minute
  if (currentMillis - previousMillis >= UPDATE_INTERVAL) { 
       previousMillis = currentMillis; //Update the time field
       
       distanceData = getDistanceData();

       while (weightISR() == false){
          weightISR();
       }
      
       String createdAt = getLocalTime();
       sendToFirebase(createdAt); //Send data to firebase with the created timee
  }
}
