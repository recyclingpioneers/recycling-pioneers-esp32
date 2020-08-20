# Recycling Pioneers ESP32 Logic Files

Project underway by Recycling Pioneers with Buildings and Grounds of McGill University. 

This repository includes all the logic for the ESP32 modules that will be used for the smart bins around the Downtown McGill Campus. All sensitive information incluing database keys
and WiFi information is hidden for security reasons.

Please contact Arneet Kalra for any information. 

## Files Included

1) ESP32_Calibration: Logic to help calibrate the scale when installing.
2) ESP32_Logic_W_Intervals: Logic for bins that will read data every 30 min. 
3) ESP32_Realtime_logic: Logic needed for bins that will be using realtime data.

## Where the project is at:
The final touches are currently being put for the low-level software. Then Recycling Pioneer's focus will shift towards developing the official Recycling Pioneer website, where all the data will be presented via the Firebase Realtime Database. 

## How to install the code on an ESP32:

For most of our bins, we will be installing the ESP32_Logic_W_Intervals code on the ESP32 micocontrollers. The following lines of code must be modfiied when setting up these micocontrollers:

1) Line 72: const String BIN_NAME = "{building_name}{closest_room}"; 
Change the name of the sensor to "{building_name}{closest_room}". For example, our first bin is "trottier1050".

2) Line 73-76 
const String BUILDING_NAME = "";
const String FACULTY_NAME = "";
const int FLOOR_NUMBER = ;
const String CLOSEST_ROOM = "";

Update these values for the bin in which you are doing the installations. Example:
const String BUILDING_NAME = "TROT";
const String FACULTY_NAME = "ECSE";
const int FLOOR_NUMBER = 1;
const String CLOSEST_ROOM = "1050";

3) Using the ESP32_Calibration file, find the calibration value for the load cell amplifier you are using. Reminder: Each weight sensor has its own load cell amplifier. For a McGill bin with 4 compartments, 4 different load cell amplifiers will be used for each compartment (plastic, waste, recycling, compost). Instructions on how to use the ESP32_Calibration file are found in its documentation. 

4) The calibration values are listed on lines 100-103. Depending on the compartment in which the load cell amplifier has to be installed, place the calibration value on its specific line. For example, if you are installing a load cell amplifier in the recycle compartment of a bin with calibration value 20764, you would put the calibration value on line 103 as "float calibrationValue_RECYCLE = 20764.00;"

5) Also, you have to ensure you have the hidden keys, as shown in line 24-28. Please contact Arneet Kalra if you don't. 

That's it! Upload your code to the board and you're done. Data will be read from the sensors and get uploaded to the Firebase Realtime Database. 

### Other Documentation:

1) As a team, we chose to have 10 minute intervals for reading data. If this need to be changed at any point, it can be done so on line 69 ("#define UPDATE_INTERVAL 600000 //Time in milliseconds between updating firebase database (10 min)"). 10 min in milliseconds is 600000. To change this number, find the new time in milliseconds and update. 

2) The pins for each sensor have been laid out between lines 34 and 52. If at any point these sensors have to be hooked onto different GPIO pins, it needs to be reflected there. However, these pins have been preselected and work well, so it's best to be consistent.

3) The time is calculated using an NTP server, lines 91-93. The offsets have been set for our timezone. If ever the time is inaccurate, verify the numbers match up here. The offset is given in seconds. In Montreal we are at -5 hours so an offset of -18000 is used. There is also a daylight saving of 3600 which is used primarily everywhere.
