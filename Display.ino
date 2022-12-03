
#include <SPI.h> 
#include <Wire.h>

#include <Adafruit_SSD1306.h>


#define therm_power 11 // when ever "therm_power" is read 11 is placed
#define display_power 9
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 
#define SCREEN_ADDRESS 0x3C
int ThermistorPin = 0;
int Vo;
float R1 = 57000; //value of resistor in series with thermistor
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07; // steinhart-harts constance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
Serial.begin(9600); // 9600 updated bits/s

pinMode(therm_power, OUTPUT); // Sets PIN 11 as an output
pinMode(display_power, OUTPUT);
digitalWrite(display_power, HIGH);
display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
//  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
//    Serial.println(F("SSD1306 allocation failed"));
//    for(;;); // Don't proceed, loop forever
//  }

 
}

void loop() {
  Serial.print("Here");

  delay(10);
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Weather Station"));
  display.setCursor(0,9);             // Start at top-left corner
  display.print("Temperature: ");
  display.setCursor(80,9); 
  display.print(T);
  display.setCursor(0,20); 
  display.print("Humidity: ");
  display.display();
 
 
}

void weather_station() {
   digitalWrite(therm_power, HIGH); //suppies voltage to pin11/thermistor
  

  Vo = analogRead(ThermistorPin); //Vo is Vout the voltage at the thermistor
  R2 = R1 * (1023.0 / (float)Vo - 1.0); //solving for the thermistor resistance, equation shown in report
  logR2 = log(R2); //find the log of the resistor value
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // Calculated temperature in KELVIN
  T = T - 273.15; // Converts it to Celcius
  T = (T * 9.0)/ 5.0 + 32.0; //converts from celcius to faranheit

  Serial.print("Temperature: "); 
  Serial.print(T); //prints out the temperature in serial port
  Serial.println(" F");
  
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Weather Station"));
  display.setCursor(0,9);             // Start at top-left corner
  display.print("Temperature: ");
  display.setCursor(80,9); 
  display.print(T);
  display.setCursor(0,20); 
  display.print("Humidity: ");
  display.display();
 
  
}
