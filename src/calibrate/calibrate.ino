#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
DS3231 rtc;

void setup() {
  // start serial
  Serial.begin(9600);
  while(!Serial);
  // start I2C
  Wire.begin();

  // set RTC clock to correct time (be sure its set 6s ahead to the correct time when flashing)
  rtc.setClockMode(false);  // use 24h format
  rtc.setYear(24);          // 2024
  rtc.setMonth(4);          // April
  rtc.setDate(8);           // 4th
  rtc.setHour(3);
  rtc.setMinute(9);
  rtc.setSecond(5);
  Serial.println("Clock set!");

  // set IR emissivity
  mlx.begin();
  mlx.writeEmissivity(0.95);
  Serial.println("IR set!");
}

void loop() { }
