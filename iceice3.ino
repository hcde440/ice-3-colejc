#include <Adafruit_MPL115A2.h> //pressure
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h> //display
#include <Adafruit_GFX.h> //display
#include "config.h" //IO
#include <Adafruit_Sensor.h> //temp and hum
#include <ESP8266WiFi.h> //wifi
#include <DHT.h> //temp and hum
#include <DHT_U.h> //temp and hum


//ICE 3
//Sensing and displaying temp, humidity, and pressure information to an LCD and Adafruit IO board
//IO board at: https://io.adafruit.com/colej5/dashboards/ice3

// pin connected to DH22 data line
#define DATA_PIN 12

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //declare display

// create DHT22 instance
DHT_Unified dht(DATA_PIN, DHT22);

// set up the 'temperature','humidity', and 'pressure' feeds
AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *pressure = io.feed("pressure");

//declare pressure sensor
Adafruit_MPL115A2 mpl115a2;

void setup() {

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  // initialize dht22
  dht.begin();

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  //Start pressure sensor
  mpl115a2.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  //Grab some data from our temperature sensor
  sensors_event_t event;
  dht.temperature().getEvent(&event);

  //Set the values of celsius and fahrenheit using our reading
  float celsius = event.temperature;
  float fahrenheit = (celsius * 1.8) + 32;

  //Print temperature information to Serial
  Serial.print("celsius: ");
  Serial.print(celsius);
  Serial.println("C");
  Serial.print("fahrenheit: ");
  Serial.print(fahrenheit);
  Serial.println("F");

  //Clear the LCD display, set our cursor, and output the information taken from sensors to the LCD
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("temp: ");
  display.print(celsius);
  display.print(F("C, "));
  display.print(fahrenheit);
  display.println(F("F"));

  //Save fahrenheit info to Adafruit IO
  temperature->save(fahrenheit);

  //Grab humidity data from our sensor
  dht.humidity().getEvent(&event);

  //Output humidity data to Serial
  Serial.print("humidity: ");
  Serial.print(event.relative_humidity);
  Serial.println("%");

  //Display humidity data on LCD
  display.print("humidity: ");
  display.print(event.relative_humidity);
  display.println(F("%"));

  // save humidity to Adafruit IO
  humidity->save(event.relative_humidity);

  //Get the pressure information from our sensor
  float pressureKPA = mpl115a2.getPressure(); 
  
  //Output the pressure data to Serial 
  Serial.print("Pressure (kPa): "); Serial.print(pressureKPA, 4); Serial.println(" kPa");

  //Display pressure data on our LCD
  display.print("pressure: ");
  display.print(pressureKPA);
  display.println(F(" kPA"));

  //Save pressure data to Adafruit IO
  pressure->save(pressureKPA);

  //Turn on the LCD display
  display.display();

  // wait 5 seconds (5000 milliseconds == 5 seconds)
  delay(5000);

}
