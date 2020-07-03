// SMART IRRIGATION SKETCH

// Including relevant libraries
#include <ThingSpeak.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

// Local Network Settings
byte mac[] = {
  0xA8, 0x61, 0x0A, 0xAE, 0x78, 0xDF
};
byte ip[] = { 192,168, 1, 30 };
EthernetClient client;

// DS1302 RTC settings
// CLOCK CONNECTIONS:
// DS1302 CLK/SCLK --> 5
// DS1302 DAT/IO --> 4
// DS1302 RST/CE --> 2
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND
ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// ThingSpeak Settings
unsigned long MyIoT = 1064859;
const char * writeAPIKey = "V1C4YQA926ZX48H4";

// Defining variables
// Moisture
#define MoisturePin A0 // output pin
float sensorInput_moisture = 0;   
float moisture; 
int moisture_percentage;
     
// Temperature
#define TemperaturePin A3 // output pin
int sensorInput_temp = 0;
float temperature;

// pump settings
byte pump = 7; // pump activated through relay connected to the 7th pin

// LED settings
byte ledPin = 6;

void setup() {
  // Setup code
  // Serial monitor
  Serial.begin(9600);

  // Start Ethernet on Arduino and ThingSpeak communication
  Serial.println("Setting up...");
  Ethernet.begin(mac);
  ThingSpeak.begin(client); 
  Serial.println("Ready to start");

  // RTC adjustments
  Rtc.Begin(); // start the RTC

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__); 
  printDateTime(compiled);

  RtcDateTime now = Rtc.GetDateTime(); // setting time from PC
  if (now < compiled) 
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
 
  // output pump
  pinMode(pump, OUTPUT); // variant low/high
  
  // Output LED
  pinMode(ledPin,OUTPUT); // 6th pin powers LED
}

void loop() {
  // Loop code
  digitalWrite(pump, HIGH); // pump deactivated
  digitalWrite(ledPin, HIGH); // turn on LED during data transfer
  RtcDateTime now = Rtc.GetDateTime(); // get time
  
  // Finding moisture
  sensorInput_moisture = 0;
  for (int i = 0; i <= 100; i++) 
  { 
    sensorInput_moisture = sensorInput_moisture + analogRead(MoisturePin); // taking average of 100 sensor inputs for better accuracy
    delay(1); 
  } 
  moisture = sensorInput_moisture/100.0; // moisture is the average of the 100 readings above
  if (moisture >= 1024) {  // moisture max is 1024 (from readings)
    moisture = 1024;
  }
   if (moisture <= 0) {  // moisture min is 0 (from readings)
    moisture = 0;
  }
  moisture_percentage = map(moisture,0,1024,0,100); // mapping so we have a % value for the moisture
  Serial.print("Current Moisture: "); // printing moisture
  Serial.print(moisture_percentage); 
  Serial.println("%");  

  // Finding temperature
  sensorInput_temp = analogRead(TemperaturePin);    // read the analog sensor and store it
  temperature = (float)sensorInput_temp / 1024;       // find percentage of input reading
  temperature = temperature * 5;                 // multiply by 5V to get voltage
  temperature = temperature - 0.5;               // Subtract the offset 
  temperature = temperature * 100;               // Convert to degrees celcius
  
  Serial.print("Current Temperature: ");  // printing temperature
  Serial.print(temperature);
  Serial.println("°C");

  // Sending data to ThingSpeak
  ThingSpeak.writeField(MyIoT, 1, moisture_percentage, writeAPIKey);  // sending moisture data
  digitalWrite(ledPin, LOW); // turn off LED during downtime
  Serial.println("Sending moisture data to ThingSpeak... ");
  printDateTime(now); // print time of data transfer
  delay(30000); // send data every 30 seconds

  now = Rtc.GetDateTime(); // get time
  digitalWrite(ledPin, HIGH); // turn on LED during data transfer
  ThingSpeak.writeField(MyIoT, 2, temperature, writeAPIKey);
  digitalWrite(ledPin, LOW); // turn off LED during downtime
  Serial.println("Sending temperature data to ThingSpeak... ");
  printDateTime(now); // print time of data transfer

  // Watering  
  checkWateringConditions(now,moisture_percentage);
  delay(20000); // send data every 30 seconds (10 second delay in checkWateringConditions()

  // Watering (TEST VERSION FOR VIDEO) 
  //digitalWrite(pump, LOW); // activate pump for 10 seconds
  //delay(10000);
  //digitalWrite(pump, HIGH); // deactivate pump
  //delay(20000); // 20 second delay before sending data again
}

// function that prints time, according to RTC
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    // saving datetime in char array
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    // printing full time format
    Serial.print(datestring);
    Serial.println();
}

// function that checks whether plant should be waterred or not
void checkWateringConditions(const RtcDateTime& dt, int moisture_percentage)
{
    // getting time
    char datestring[20];

    // keeping only hour and minute 
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u:%02u"),
            dt.Hour(),
            dt.Minute());
            
   // if time is 11:00 exactly, check for moisture
    if (strcmp(datestring,"11:00")==0) {
      Serial.println("Checking moisture...");
      if (moisture_percentage < 70) { // water plant if moisture is less than 70%
        Serial.println("Watering plant!");
        digitalWrite(pump, LOW); // activate pump to water plant
      } else {  // moisture is above or equal to 70%, so we can assume plant is watered
        Serial.println("Soil moisture above 70%, watering plant is not necessery.");
        
      }
    } 

    // 10 second delay
    delay(10000);
    digitalWrite(pump, HIGH); // deactivate pump if it's activated
}
