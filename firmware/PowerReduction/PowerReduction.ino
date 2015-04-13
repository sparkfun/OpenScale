/*
 OpenScale: A serial interface for reading and configuring load cells.
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 24th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example code uses bogde's excellent library: https://github.com/bogde/HX711
 SparkFun spends a lot of time and energy building open source hardware and writing public domain code.
 Please consider supporting SparkFun by buying a product or kit.

 OpenScale is a simple board that allows a user to read and configure all types of load cells.
 It relies on the HX711 load cell amplifier.

 How to use:
 1) Wire your load cell to the board using the 4-pin connection (E+/-, A+/-) or the RJ45 connection.
 2) Attach board to USB and open terminal at 9600bps
 3) Press x to bring up settings menu
 4) Select units LBS/KG
 5) Tare the scale with no weight on the scale
 6) Calibrate the scale: Remove any weight, start calibration routine, place weight on scale, adjust calibration
 factor until scale reads out the calibration weight.
 7) Press x and test your scale

 OpenScale ships with an Arduino/Optiboot 115200bps serial bootloader running at 16MHz so you can load new firmware
 with a simple serial connection. Select 'Arduino Uno' under the boards menu to reprogram the board.

 OpenScale runs at 9600bps by default. This is configurable to 1200, 2400, 4800, 9600, 19200, 38400, 57600, and 115200bps.

 After power up OpenScale will try reading the load cell and output a weight value.

 If you get OpenScale stuck into an unknown baudrate, there is a safety mechanism built-in. Tie the RX pin
 to ground and power up OpenScale. You should see the status LED blink at 1Hz for 2 seconds. 
 Now power down OpenScale and remove the RX/GND jumper. OpenScale is now reset to 9600bps.

 To change the baud rate type 'x' to bring up configuration menu. Select the baud rate sub menu and enter
 the baud rate of your choice. You will then see a message for example 'Going to 9600bps...'. 
 You will need to power down OpenScale, change your system UART settings to match the new OpenScale
 baud rate and then power OpenScale back up.

 STAT LED / D13 - toggles after each report

 If you're using this firmware with the HX711 breakout and an Uno here are the pins to hook up:
 Arduino pin 2 -> HX711 CLK
 3 -> DAT
 5V -> VCC
 GND -> GND

 TODO:
 Change the read range from 100s of lbs to grams or micrograms.
 Make zero range changeable by user
 Test scale from 1 coke to double coke
 Allow for direct/raw output

 Testing:
 10.0968 - 8:20AM
 10.4319 - 3:05PM

 Phidgets moved 0.4lbs over 24hrs
 10.1 8AM
 10.44 8:45PM
 10.55 7:40AM

 Scale after coke on it for 24 hours
 10.06 7:45AM
 9.97 8:12AM
 9.96 7:58AM the following morning
 Looks like we can get to a static point. Seems to be... break in? on the load cell

 3/28
 10.1 at 8:50AM calibrated
 9.83 at 9AM
 10.1 at 3:37PM re-calibrated
 9.39 at 6:19pm, recaled to 10.1
 9.67 at 9:19pm, recaled to 10.1
 3/29 
 8.94 at 8:27AM
 
 3/31 wooden scale
 8:27am cal (96530) to 10.1
 8PM perfect 10.10. Adding 2nd coke, 20.24 (really good)
 
 Need to duplicate with ehternet cable
 
 Testing resistors:
 wooden: grn/white and red/black are 1k. Every other combo is 750
 metal: grn/white and red/blk are 2k. Every other combo is 1.5k
 
 3/31 More testing with metal scale
 9:45PM cal (-6353) to 10.1
 
 4/2 Testing with wooden + ehternet cable
 8:34AM cal to 10.1 (-96179)
 
 4/3 Testing with wooden + ehternet cable
 7PM 9.89 seems pretty close
 recal to 10.1 (-93261)
 9:30AM 10.07. Nice. Works!
 
 Ethernet cable to wooden works after a few hour cal
 
 
 
 */

#include "HX711.h" //Library created by bogde
#include "openscale.h" //Contains EPPROM locations for settings
#include <Wire.h> //Needed to talk to on board TMP102 temp sensor
#include <EEPROM.h> //Needed to record user settings
#include <OneWire.h> //Needed to read DS18B20 temp sensors
#include "LowPower.h" //https://github.com/rocketscream/Low-Power
//15 ms, 30 ms, 60 ms, 120 ms, 250 ms, 500 ms, 1 s, 2 s, 4 s, 8 s, and forever

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers


#define FIRMWARE_VERSION "1.0"

const byte statusLED = 13;  //Flashes with each reading

HX711 scale(DAT, CLK); //Setup interface to scale

void setup()
{
  pinMode(statusLED, OUTPUT);

  //During testing reset everything
  //for(int x = 0 ; x < 30 ; x++)
  //{
  //  EEPROM.write(x, 0xFF);
  //}
  
  Wire.begin();

  //Power down various bits of hardware to lower power usage  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();

  //Shut off Timer2, Timer1, ADC
  ADCSRA &= ~(1<<ADEN); //Disable ADC
  ACSR = (1<<ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins
  DIDR1 = (1<<AIN1D)|(1<<AIN0D); //Disable digital input buffer on AIN1/0

  power_timer1_disable();
  power_timer2_disable();
  power_adc_disable();
  power_spi_disable();

  pinMode(AMP_EN, OUTPUT);
  digitalWrite(AMP_EN, LOW); //Turn on power to HX711

  //Setup UART
  Serial.begin(9600);


  scale.power_down();

  scale.set_scale(-4000); //Calibrate scale from EEPROM value
  scale.set_offset(300); //Zero out the scale using a previously known zero point

  
}


void loop()
{
  Serial.println(F("Readings:"));
  delay(1000);
  
  LowPower.idle(SLEEP_1S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
                SPI_OFF, USART0_OFF, TWI_OFF);

}


