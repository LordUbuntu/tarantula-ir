/*
MIT License

Copyright (c) 2024 Jacobus Burger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* Credits:
 *  Project Commission and Project Specifications by Nicholas Sakich (2024)
 *  This Software and Electronic Hardware by Jacobus Burger (2024)
 * Description:
 *  This is a program for an IR temperature data logger to be used by
 *    Nichoals Sakich on his tarantula study down in Peru this April.
 * Features:
 *  - Measures data using a directed IR thermopile
 *  - Records precise time with an RTC module
 *  - Logs data with a uSD module
 *  - Utilizes an indicator LED for field debugging
 * Parts Used:
 *  - Arduino Nano (ATMEGA382p) MCU
 *    - https://content.arduino.cc/assets/Pinout-NANO_latest.pdf
 *  - MLX90614 IR
 *    - https://www.digikey.ca/en/products/detail/melexis-technologies-nv/MLX90614ESF-BAA-000-TU/1647941
 *  - DS3231 RTC
 *    - https://www.digikey.ca/en/prodlucts/detail/analog-devices-inc-maxim-integrated/DS3231S-T-R/1197575
 *  - ADA254 uSD
 *    - https://www.adafruit.com/product/254
 *  - TPS61023 MiniBoost
 *    - https://www.adafruit.com/product/4654
 *  - Adafruit 64MB and 128MB uSD cards
 *    - https://www.adafruit.com/product/5249
 *    - https://www.adafruit.com/product/5250
 * Libraries Used:
 *  MLX90614 Library v2.1.5 (With Adafruit BusIO v1.15.0): https://github.com/adafruit/Adafruit-MLX90614-Library
 *  DS3231 Library v1.1.2: https://github.com/NorthernWidget/DS3231
 *  SD Library v1.2.4: https://www.arduino.cc/reference/en/libraries/sd/
 * Space Used:
 *  program storage: 20760/30720 bytes
 *  dynamic memory: 1501/2048 bytes 
 */
#include <Wire.h>                 // I2C interfacing
#include <DS3231.h>               // RTC
#include <Adafruit_MLX90614.h>    // IR
#include <SPI.h>                  // SPI interfacing
#include <SD.h>                   // SD card
// ***** INDICATOR *****
#define LED 7   // indicator LED connected on pin D7
// ***** RTC *****
DS3231 rtc;
// These booleans are quirks of the DS3231 library, see the library documentation for more information.
bool century_bit;
bool h12 = true;
bool hPM = false;
// Used to print a timestamp in the format YYYY-MM-DD HH:MM:SS to get a measure of the current time in PET (PEru Time).
// I assume we're in the 20XX when this is being run, if not hello from the past!!!
String print_time() {
  String date = "20" + String(rtc.getYear());             // YYYY, 20XX
  date = date + "-" + String(rtc.getMonth(century_bit));  // YYYY-MM
  date = date + "-" + String(rtc.getDate());              // YYYY-MM-DD
  date = date + " ";
  date = date + String(rtc.getHour(h12, hPM));            // YYYY-MM-DD HH
  date = date + ":" + String(rtc.getMinute());            // YYYY-MM-DD HH:MM
  date = date + ":" + String(rtc.getSecond());            // YYYY-MM-DD HH:MM:SS
  return date;
}
// ***** IR *****
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
// ***** SD CARD *****
#define CS 10                                                             // Define the slave select (SS) associated pin 10 as chip select (CS) pin for uSD card library.
File datalog;                                                             // data is logged to a single csv file on the chip
#define CSV_HEADER "Time (PET),Object Temperature,Ambient Temperature"    // The CSV header for the datalog.csv file


void setup() {
  // set and turn on indicator LED to show that setup has begun
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // Start serial communications line and define baud rate
  Serial.begin(9600);
  while (!Serial);  // wait for serial to initialize

  // Start I2C interface to communicate with RTC and IR modules
  // over Serial DAta (SDA) and Serial CLock (SCL) lines.
  Wire.begin();

  // test RTC module
  Serial.print("Testing RTC module (DS3231)...");
  if (rtc.getTemperature() <= -9999) {  // -9999 is an error return for the builtin temperature sensor
    Serial.println("FAIL!");            // which indicates that the module isn't present/working.
    while(true);
  } else {
    Serial.println("DONE!");
  }
  Serial.print("Time = ");
  Serial.println(print_time());

  // test IR module
  Serial.print("Initializing IR module (MLX90614)...");
  if (!mlx.begin()) {
    Serial.println("FAIL!");
    while(true);
  } else {
    Serial.println("DONE!");
  }
  Serial.print("Emissivity = ");
  Serial.println(mlx.readEmissivity());

  // test  uSD module
  // The SS pin must be held high to ensure master SPI operation
  //  so that it can operate on CS pin of SD card module.
  // This can be thought of as initializing the SPI interface.
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  // Check uSD module is there
  Serial.print("Initializing uSD module (ADA254)... ");
  if (!SD.begin(CS)) {
    Serial.println("FAIL!");
    while(true);
  } else {
    Serial.println("SUCCESS!");
  }
  // Write the header line to the CSV file if it doesn't exist yet
  if (!SD.exists("datalog.csv")) {
    datalog = SD.open("datalog.csv", FILE_WRITE);
    datalog.println(CSV_HEADER);
    datalog.close();
    Serial.print("Wrote header to datalog: ");
    Serial.println(CSV_HEADER);
  }
  // Check that datalog.csv is accessible
  datalog = SD.open("datalog.csv", FILE_WRITE);
  if (!datalog) {
    Serial.println("datalog.csv file is NOT accessible!");
    while(true);
  } else {
    Serial.println("datalog.csv file is accessible!");
  }
  datalog.close();

  // signal that setup is completed
  delay(1000);
  digitalWrite(LED, LOW);
  Serial.println("***** INITIALIZATION COMPLETE! *****");
}


void loop() {
  // Collect sensor data as strings.
  String time = print_time();
  String ambient_temp = String(mlx.readAmbientTempC());
  String object_temp = String(mlx.readObjectTempC());
  // Print sensor data to serial.
  String data = time + "," + object_temp + "," + ambient_temp;
  char data_str[32]{};
  strcpy(data_str, data.c_str());
  Serial.println(data_str);
  // Ensure that the SD card is detected before continuing to the logging step.
  // To an outside observer this will appear like continuous blinking every second.
  while (!SD.begin(CS)) {
    Serial.println("No uSD card detected!");
    delay(1000);
  }
  // Open the SD card datalog.csv file, log data, and then close the file to avoid corruption.
  datalog = SD.open("datalog.csv", FILE_WRITE);
  if (datalog) {
    datalog.println(data_str);
    Serial.println("wrote to datalog.csv");
  } else {
    Serial.println("Could not open datalog.csv!");
    // signal with indicator that a write failed
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
  }
  datalog.close();
  // Wait some time before the next log.
  delay(60000);
}