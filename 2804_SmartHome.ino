
#include <Talkie.h>
#include "Vocab_US_Large.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Thermistor.h>
#include <Humidity.h>
#include <SineGenerator.h>
#include <Timer.h>


#define COFFEE 5 // voltage supply for the photo sensor
#define NIGHTLIGHT 4
#define AC_UNIT 6
#define DISPLAY 9
#define TRIGGER_PIN  10 // whenever "TRIGGER_PIN" is read, 10 is placed
#define therm_power 11 // when ever "therm_power" is read, 11 is placed
#define WATER 12
#define ECHO_PIN     13 // whenever "ECHO_PIN" is read, 13 is placed
#define MAX_DISTANCE 13.1234 //HCSRO4 sensor max and min values in feet
#define MIN_DISTANCE 0.065

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // const needed for the display library to work
#define SCREEN_ADDRESS 0x3C //i^2C address for usig the display
#define Vin 1023 //used to represent 5 volts in NIGHLIGHT equation


#define water_min 40
#define water_max 60
#define PHOTO_ANALOG 3
#define SOIL_READING 2
#define AUTO_OFF 'A'
#define AUTO_ON 'a'


float duration, distance; // variables used for the intrusion device
int ThermistorPin = 0; // uses the analog pin to read temp
int therm_Vo; // analog reading
float R1 = 57000; //value of resistor in series with thermistor
float logR2, R2, T;//declares variables to later be using in steinhart-hart's equation
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07; // steinhart-harts constants
bool manualAC = false; // true if ON button is pressed on GUI
bool manualWater = false; //true if ADD button is pressed on GUI
String AC = "AUTO";
char signal_r; //char being recieved from the GUI

int sensor_val; // value of
int soil_moist; // relative humidity of the soil
const int dry = 570; // sensor value in dry soil
const int wet = 840;// sensor value in wet soil

int RH =1;
int temp;
Talkie voice;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // initiates the OLED display

Humidity Humidity(0);
Thermistor Temp(1);
SinGen Sine(9);
SWTimer Program(5000);  //constructor for program timer
SWTimer HumidityTime(10000);   //constructor for humidity timer

void setup() {
  Serial.begin(9600);   //initialize the baud rate

  voice.doNotUseInvertedOutput();
  pinMode(COFFEE, OUTPUT);
  pinMode(NIGHTLIGHT, OUTPUT); //LED to test condition of intrusion device
  pinMode(therm_power, OUTPUT); // makes pin 11 a voltage supply
  pinMode(TRIGGER_PIN, OUTPUT); // makes pin 13 a voltage supply
  pinMode(ECHO_PIN, INPUT); //makes pin 10 an input
  pinMode(DISPLAY, OUTPUT); //makes pin 9 a voltage suppy
  pinMode(WATER, OUTPUT); //sets pin 12 as an output
  pinMode(AC_UNIT, OUTPUT); //sets pin 6 as an output for AC UNIT control

   


  digitalWrite(DISPLAY, HIGH); //voltage supply to display

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); //begins communication with the OLED display


  Humidity.SetUp();         //constructor for humidity sensor libray
  Temp.SetUp();             //consructor for temp library
  Sine.SetUp(100, 2.5);      //constructor for sin wave generator library

}

void loop() {

  if (Serial.available() > 0) {
    // read the incoming byte
    signal_r = Serial.read();
  }

  Program.SWTimer_start();      //starts program timer
  if (Program.SWTimer_expired())  //checks to see if program timer
  {
    Program.SWTimer_reset();      //resets program timer
    bool Delay = true;         //delay condition for humidity sensor loop
    while (Delay) {
      HumidityTime.SWTimer_start();           //starts humidity timer
      if (HumidityTime.SWTimer_expired())     //checks to see if humidity timer is expired
        Delay = false;                    //sets the delay to false if it is breaking the loop
        humidity();                         //checks humidity
    }
  }
  else {
    //device always active / updating their status
    intrusion_device();
    weather_station();
    nightLight();
    plantWater();
    coffeeMachine();
    delay(500);
  }
}



void intrusion_device() {
  digitalWrite(TRIGGER_PIN, LOW); //initialized the pulse
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);//10 microsecond voltage pulse

  duration = pulseIn(ECHO_PIN, HIGH); // time taken to recieve value from the ECHO pin

  distance = (duration/2)*0.001148; //time measured is twice the distance taken to get there
  //intrusion device that detects object within 8 feet and displays it on GUI and the OLED
  if ((distance >= MAX_DISTANCE) | (distance <= MIN_DISTANCE)) {
    //    Serial.println("OUT OF RANGE");
  }
  //following distance is in feet
  else if (distance < 8) {
    Serial.println("INTRUDER");
    Serial.println("DETECTED");
    
    voice.say(sp2_DANGER);
    voice.say(sp2_DANGER);
    voice.say(sp2_DANGER);
         
    
  }
  else {
    Serial.println("CLEAR");
  }
}

void humidity()
{
  temp = Temp.Update();
  temp += 5;
  //temp = (((temp - 32)*5)/9);
  RH = Humidity.Update(temp);
  Sine.Update();
  Serial.print("h");
  Serial.print(RH);
  Serial.println("%RH");
}

void weather_station()
{
  digitalWrite(therm_power, HIGH); //suppies voltage to pin11/thermistor


  T = Temp.Update();              //updates temperature

  T +=5;

  //AC is initalized to AUTO mode
  //ON button sigal recieved, AC in manual mode
  if (signal_r == AUTO_OFF) {
    digitalWrite(AC_UNIT, HIGH);
    manualAC = true;
   
  }
  //AUTO button signal recieved, AC back to auto mode
  else if (signal_r == AUTO_ON) {
    digitalWrite(AC_UNIT, LOW);
    manualAC = false;
  }
  else {
  }
  //following temperatures are in degrees Fahrenheit
  if (manualAC == false) {
    Serial.println("AUTO");
    if (T > 78) {
      digitalWrite(AC_UNIT, HIGH);
    }
    if (T <= 78) {
      digitalWrite(AC_UNIT, LOW);
    }
  }
  //displays the temperature and humitiy on display
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.println(F("Weather Station"));
  display.setCursor(0, 9);            // Start at top-left corner
  display.print("Temperature: ");
  display.setCursor(80, 9);
  display.print(T);
  display.setCursor(0, 20);
  display.print("Humidity: ");
  display.print(RH);
  display.display();

  Serial.print("P"); //prints P to parse for char in processing
  Serial.print(T);
  Serial.println(" F");
}

void nightLight() {

//  int Vout = Vin - analogRead(PHOTO_ANALOG); // Creates a value for vout
  int Vout = analogRead(PHOTO_ANALOG); // Creates a value for vout
  Serial.print("Nightlight: ");
  Serial.println(Vout);

  if (Vout < (Vin * .4)) {
    digitalWrite(NIGHTLIGHT, HIGH); //when the PHOTOCELL VOLTAGE DROP >2.5V LED will turn on
  }
  else {
    digitalWrite(NIGHTLIGHT, LOW); //for any other condition it will turn off
  }

}
void plantWater() {

  sensor_val = 1023 - analogRead(SOIL_READING); //analog values decrease as capacitance increases, this inverts the way it's increments
  soil_moist = map(sensor_val, wet, dry, 100, 0); //gives us the humidity of the soil as a percentage 100 = saturated soil
  //  Serial.println(sensor_val); // used for debugging
  //we know that house plants require 40-60% humidity in soil so we can set conditions for watering system
  Serial.print(soil_moist);
  Serial.println("%RH");
  //add button is pressed, now in manual watering mode
  if (signal_r == 'W') {
    digitalWrite(WATER, HIGH);
    manualWater = true;
  }
  //STOP button is pressed, now back in auto mode
  if (signal_r == 'w') {
    digitalWrite(WATER, LOW);
    manualWater = false;
  }

  if (manualWater == false) {
    if ((soil_moist <= water_max) & (soil_moist >= water_min)) {
      //    Serial.print(soil_moist); // used for debugging
      digitalWrite(WATER, LOW);

    }
    else if (soil_moist > water_max) {
      digitalWrite(WATER, LOW);
      //    Serial.println("DRENCHED");
    }
    else {

      //    Serial.println("DRY");
      digitalWrite(WATER, HIGH);
    }

  }
}

void coffeeMachine() {
  if (signal_r == '1') {
    digitalWrite(COFFEE, HIGH);
  }

  if (signal_r == '0') {
   digitalWrite(COFFEE, LOW);
  }
}
