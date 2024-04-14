# Tarantula IR Sensor
This is an Arduino Nano project done for Nicholas Sakish. The goal was to create a device which measures the infrared temperature of an object from a distance and logs the temperature and time data every minute to a microSD card.

## Code
See the `src/` directory in this repository.

## Hardware
If you want to make a similar project here are the components and schematic to do so.

### Components
- Arduino Nano V3
- DS3231 RTC
- MLX90614ESF-BAA IR Thermometer
- ADA254 MicroSD Card Breakout
- TPS61023 MiniBooster 5V@1A
- Adafruit Class 4 MicroSD cards (<= 128**MB**)

### Schematic
![Diagram](https://github.com/LordUbuntu/tarantula-ir/blob/main/diagram.png)
