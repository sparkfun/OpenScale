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


  Firmware versions:
  v1.0 - Original release
  v1.1 - Added trigger character
  v1.2 -

  TODO:
  - on keypad doesn't work
  powering the scale down seems to be causing the raw readings to be off
  replace EEPROM reads/writes with gets/puts
  Echo calibration user entry
  
*/

#include "HX711.h" //Original Repository Created by Bodge https://github.com/bogde/HX711
#include "openscale.h" //Contains EPPROM locations for settings
#include <Wire.h> //Needed to talk to on board TMP102 temp sensor
#include <EEPROM.h> //Needed to record user settings
#include <OneWire.h> //Needed to read DS18B20 temp sensors

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers

#define FIRMWARE_VERSION "1.2"

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
long setting_uart_speed; //This is the baud rate that the system runs at, default is 9600. Can be 1,200 to 1,000,000
byte setting_units; //Lbs or kg?
unsigned int setting_report_rate;
long setting_calibration_factor; //Value used to convert the load cell reading to lbs or kg
long setting_tare_point; //Zero value that is found when scale is tared
boolean setting_timestamp_enable; //Prints the number of miliseconds since boot next to weight reading
byte setting_decimal_places; //How many decimals to display
byte setting_average_amount; //How many readings to take before reporting reading
byte setting_local_temp_enable; //Prints the local temperature in C
byte setting_remote_temp_enable; //Prints the remote temperature in C
byte setting_status_enable; //Turns on/off the blinking status LED
byte setting_serial_trigger_enable; //Takes reading when serial character is received
byte setting_raw_reading_enable; //Prints the raw, 24bit, long from the HX711, ex: 8355808
byte setting_trigger_character; //The character that will cause OpenScale to report a reading
boolean setupMode = false; //This is set to true if user presses x

const byte escape_character = 'x'; //This is the ASCII character we look for to break reporting
const int minimum_powercycle_time = 500; //Anything less than 500 can cause reading problems
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

const byte statusLED = 13;  //Flashes with each reading

HX711 scale(DAT, CLK); //Setup interface to scale

OneWire remoteSensor(4);  //Setup reading one wire temp sensor on pin 4 (a 4.7K resistor is necessary)
byte remoteSensorAddress[8];
boolean remoteSensorAttached = false;

void setup()
{
  pinMode(statusLED, OUTPUT);

  //Power down various bits of hardware to lower power usage
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  //Shut off Timer2, Timer1, ADC
  ADCSRA &= ~(1 << ADEN); //Disable ADC
  ACSR = (1 << ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins
  DIDR1 = (1 << AIN1D) | (1 << AIN0D); //Disable digital input buffer on AIN1/0

  power_timer1_disable();
  power_timer2_disable();
  power_adc_disable();
  power_spi_disable();

  //During testing reset everything
  //for(int x = 0 ; x < 30 ; x++)
  //  EEPROM.write(x, 0xFF);

  Wire.begin();

  readSystemSettings(); //Load all system settings from EEPROM

  //Setup UART
  Serial.begin(setting_uart_speed);
  displaySystemHeader(); //Product title and firmware version

  checkEmergencyReset(); //Look to see if the RX pin is being pulled low

  scale.set_scale(setting_calibration_factor);
  scale.set_offset(setting_tare_point);

  //Calculate the minimum time between reports
  int minTime = calcMinimumReadTime();
  Serial.print(F("Minimum time between reports: "));
  Serial.println(minTime);

  //Look for a special case where the report rate time is less than the allowed minimum
  if (setting_report_rate < minTime) setting_report_rate = minTime;

  Serial.print(F("Press "));
  Serial.print((char)escape_character);
  Serial.println(F(" to bring up settings"));

  Serial.println(F("Readings:"));

//Something odd with power cycling
  powerUpScale();
  delay(100); //Something odd with power cycling
}

void loop()
{
  //Power cycle takes around 400ms so only do so if our report rate is greater than 500ms
//Something odd with power cycling  if (setting_report_rate > minimum_powercycle_time) powerUpScale();

  long startTime = millis();

  //Take average of readings with calibration and tare taken into account
  float currentReading = scale.get_units(setting_average_amount);

  //Print time stamp
  if (setting_timestamp_enable == true)
  {
    Serial.print(startTime);
    Serial.print(F(","));
  }

  //Print calibrated reading
  Serial.print(currentReading, setting_decimal_places);
  Serial.print(F(","));
  if (setting_units == UNITS_LBS) Serial.print(F("lbs"));
  if (setting_units == UNITS_KG) Serial.print(F("kg"));
  Serial.print(F(","));

  //Print raw reading
  if (setting_raw_reading_enable == true)
  {
    long rawReading = scale.read_average(setting_average_amount); //Take average reading over a given number of times

    Serial.print(rawReading);
    Serial.print(F(","));
  }

  //Print local temp
  if (setting_local_temp_enable == true)
  {
    Serial.print(getLocalTemperature(), setting_decimal_places);
    Serial.print(F(","));
  }

  //Print remote temp
  if (setting_remote_temp_enable == true)
  {
    if (remoteSensorAttached == true)
    {
      Serial.print(getRemoteTemperature(), setting_decimal_places);
      Serial.print(F(","));
    }
    else
    {
      Serial.print(F("0,")); //There is no sensor to check
    }
  }

  if (setting_status_enable == true) toggleLED();

  Serial.println();
  Serial.flush();

  //This takes time so put it after we have printed the report
//Something odd with power cycling  if (setting_report_rate > minimum_powercycle_time) powerDownScale();

  //Hang out until the end of this report period
  while (1)
  {
    //If we see escape char then drop to setup menu
    if (Serial.available())
    {
      toggleLED();
      char incoming = Serial.read();

      if (setting_status_enable == false) digitalWrite(statusLED, LOW); //Turn off LED

      if (incoming == escape_character) 
      {
        setupMode = true;  //For Trigger Character Feature
        break; //So we can enter the setup menu
      }
    }

    if ((millis() - startTime) >= setting_report_rate) break;
  }

  //If we are serially triggered then wait for incoming character
  if (setupMode == false && setting_serial_trigger_enable == true)
  {
    //Power everything down and go to sleep until a char is received

//Something odd with power cycling    delay(100); //Give the micro time to clear out the transmit buffer
    //Any less than this and micro doesn't sleep

//Something odd with power cycling    powerDownScale();
    char incoming = 0;

    //Wait for a trigger character or x from user
    while (incoming != setting_trigger_character && incoming != 'x')
    {
      while (Serial.available() == false) {

        delay(1);
        //We  go into deep sleep here. This would save 10-20mA.
        power_twi_disable();
        power_timer0_disable(); //Shut down peripherals we don't need

        sleep_mode(); //Stop everything and go to sleep. Wake up if serial character received

        power_timer0_enable();
        power_twi_enable();
      }

      incoming = Serial.read();
      if (incoming == escape_character) setupMode = true;
    }

//Something odd with power cycling    powerUpScale();
  }

  //If the user has pressed x go into system setup
  if (setupMode == true)
  {
    //Power cycle takes 400ms so only do so if our report rate is less than 400ms
//Something odd with power cycling        if (setting_report_rate > minimum_powercycle_time) powerUpScale();
    system_setup();
//Something odd with power cycling        if (setting_report_rate > minimum_powercycle_time) powerDownScale();
    setupMode = false;

    if (setting_status_enable == false) digitalWrite(statusLED, LOW); //Turn off LED
  }
}
