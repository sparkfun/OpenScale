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
 1) Wire your load cell to the board using the 4-pin connection (E+/-, A+/-) or the RJ11 connection.
 2) Attach board to USB and open terminal at 9600bps
 3) Press q to bring up settings menu
 4) Select units LBS/KG
 5) Tare the scale with no weight on the scale
 6) Calibrate the scale: Remove an weight, start calibration routine, place weight on scale, adjust calibration 
 factor until scale reads out the calibration weight.
 7) Press x and test your scale
 
 OpenScale ships with an Arduino/Optiboot 115200bps serial bootloader running at 16MHz so you can load new firmware 
 with a simple serial connection.
 
 OpenScale runs at 9600bps by default. This is configurable to 1200, 2400, 4800, 9600, 19200, 38400, 57600, and 115200bps. 
 
 After power up OpenScale will try reading the load cell and output a weight value.
 
 If you get OpenScale stuck into an unknown baudrate, there is a safety mechanism built-in. Tie the RX pin 
 to ground and power up OpenScale. You should see the LEDs blink back and forth for 2 seconds, then blink 
 in unison. Now power down OpenScale and remove the RX/GND jumper. OpenScale is now reset to 9600bps.
 
 Type 'set' to enter baud rate configuration menu. Select the baud rate and press enter. You will then 
 see a message 'Going to 9600bps...' or some such message. You will need to power down OpenLog, change 
 your system UART settings to match the new OpenLog baud rate and then power OpenLog back up.
 
 STAT LED / D13 - flashes when character is received
 
 If you're using this firmware with the HX711 breakout and an Uno here are the pins to hook up:
 Arduino pin 2 -> HX711 CLK
 3 -> DAT
 5V -> VCC
 GND -> GND

 TODO:
 Change the read range from 100s of lbs to grams or micrograms.
 Make zero range changeable by user

 */

#include "HX711.h"
#include "openscale.h" //Contains EPPROM locations for settings

#include <EEPROM.h>

#define FIRMWARE_VERSION "1.0"

const byte statusLED = 13;  //Flashes with each reading

long setting_uart_speed; //This is the baud rate that the system runs at, default is 9600. Can be 1,200 to 1,000,000
byte setting_units; //Lbs or kg?
unsigned int setting_report_rate;
long setting_calibration_factor; //Value used to convert the load cell reading to lbs or kg
long setting_tare_point; //Zero value that is found when scale is tared 
boolean setting_time_stamp; //Prints the number of miliseconds since boot next to weight reading
byte setting_decimal_places; //How many decimals to display
byte setting_average_amount; //How many readings to take before reporting reading

byte setting_zero_window = 1; //

const byte escape_character = 'q'; //This is the ASCII character we look for to break logging

HX711 scale(DAT, CLK); //Setup interface to scale

void setup()
{
  pinMode(statusLED, OUTPUT);

  //During testing reset everything
  //for(int x = 0 ; x < 20 ; x++)
  //{
  //  EEPROM.write(x, 0xFF);
  //}

  Serial.begin(9600);
  read_system_settings(); //Load all system settings from EEPROM

  //Setup UART
  Serial.begin(setting_uart_speed);
  Serial.println("Serial Load Cell Converter");
  Serial.println("By SparkFun Electronics");
  Serial.print("Press ");
  Serial.print((char)escape_character);
  Serial.println(" to bring up settings.");

  check_emergency_reset(); //Look to see if the RX pin is being pulled low

  scale.set_scale(setting_calibration_factor); //Calibrate scale from EEPROM value
  scale.set_offset(setting_tare_point); //Zero out the scale using a previously known zero point

  //Calculate the minimum time between reports
  int minTime = calcMinimumReadTime();
  Serial.print("Minimum time between reports: ");
  Serial.println(minTime);
  
  //Look for a weird case where the report rate time is less than the allowed minimum
  if(setting_report_rate < minTime) setting_report_rate = minTime;

  Serial.println("Readings:");
}

long numberOfReadings = 0;
float lastReading = 0;
float totalReadings = 0;

void loop()
{
  long startTime = millis();

  //Print time stamp
  if(setting_time_stamp == true)
  {
    Serial.print(startTime);
    Serial.print(F(","));
  }

  //Take average of readings
  float currentReading = scale.get_units(setting_average_amount);
  
  //Zero out reading if it is too close to zero
  //if(abs(currentReading) < setting_zero_window) currentReading = 0;
  
  Serial.print(currentReading, setting_decimal_places);
  Serial.print(F(","));
  if(setting_units == UNITS_LBS) Serial.print(F("lbs"));
  if(setting_units == UNITS_KG) Serial.print(F("kg"));

  toggleLED();

  //If we see escape char then drop to setup menu
  if(Serial.available())
  {
    toggleLED();
    char incoming = Serial.read();
    if(incoming == escape_character) system_setup();
  }

  //Hang out until the end of this report period
  while( (millis() - startTime) < setting_report_rate)
    delay(1);
  
  /*float delta = lastReading - currentReading;
  lastReading = currentReading;
  
  totalReadings += delta;

  numberOfReadings++;
  
  float avgDelta = totalReadings / numberOfReadings;
  
  Serial.print(" Delta: ");
  Serial.print(delta, 3);
  Serial.print(" avgDelta: ");
  Serial.print(avgDelta, 3);*/

  Serial.println();
}


//Configure how OpenScale operates
void system_setup(void)
{
  while(1)
  {
    Serial.print(F("\r\nSerial Load Cell Converter version "));
    Serial.println(F(FIRMWARE_VERSION));
    Serial.println(F("System Configuration"));

    Serial.print(F("1) Tare scale to zero ["));
    Serial.print(setting_tare_point);
    Serial.println(F("]"));

    Serial.print(F("2) Calibrate Scale ["));
    Serial.print(setting_calibration_factor);
    Serial.println(F("]"));

    Serial.print(F("3) Time Stamp ["));
    if(setting_time_stamp == true) Serial.print(F("On"));
    if(setting_time_stamp == false) Serial.print(F("Off"));
    Serial.println(F("]"));

    Serial.print(F("4) Set report rate ["));
    Serial.print(setting_report_rate);
    Serial.println(F("]"));

    Serial.print(F("5) Set baud rate ["));
    Serial.print(setting_uart_speed);
    Serial.println(F(" bps]"));

    Serial.print(F("6) Change units of measure ["));
    if(setting_units == UNITS_KG) Serial.print(F("kg"));
    if(setting_units == UNITS_LBS) Serial.print(F("lbs"));
    Serial.println(F("]"));

    Serial.print(F("7) Decimals ["));
    Serial.print(setting_decimal_places);
    Serial.println(F("]"));

    Serial.print(F("8) Average amount ["));
    Serial.print(setting_average_amount);
    Serial.println(F("]"));

    Serial.println(F("x) Exit"));
    Serial.print(F(">"));

    //Read command
    while(!Serial.available()) ; //Wait for user to type a character
    char command = Serial.read();

    toggleLED();

    //Execute command
    if(command == '1')
    {
      //This can be used to remove the need to tare the scale.
      //Useful in permanent scale projects.
      Serial.print(F("\n\rTare point: "));

      scale.tare(); //Reset the scale to 0

      setting_tare_point = scale.read_average(10); //Get 10 readings from the HX711 and average them

      Serial.println(setting_tare_point);

      record_system_settings();
    }
    else if(command == '2')
    {
      calibrate_scale();
    }
    else if(command == '3')
    {
      Serial.print(F("\n\rTime stamp set to "));
      if(setting_time_stamp == true)
      {
        Serial.println(F("false"));
        setting_time_stamp = false;
      }
      else
      {
        Serial.println(F("true"));
        setting_time_stamp = true;
      }
      record_system_settings();
    }
    else if(command == '4')
    {
      rate_setup();
    }
    else if(command == '5')
    {
      baud_setup();
    }
    else if(command == '6')
    {
      Serial.print(F("\n\rUnits set to "));
      if(setting_units == UNITS_KG)
      {
        Serial.println(F("lbs"));
        setting_units = UNITS_LBS;
        float newFactor = (float)setting_calibration_factor * 0.453592; //Convert the calibration factor from kg to lbs
        setting_calibration_factor = (long)newFactor;
      }
      else if(setting_units == UNITS_LBS)
      {
        Serial.println(F("kg"));
        setting_units = UNITS_KG;
        float newFactor = (float)setting_calibration_factor * 2.20462; //Convert the calibration factor from lbs to kg
        setting_calibration_factor = (long)newFactor;
      }

      scale.set_scale(setting_calibration_factor); //Assign this new factor to the scale
      record_system_settings();
      
    }
    else if(command == '7')
    {
      decimal_setup();
    }
    else if(command == '8')
    {
      average_reading_setup();
    }
    else if(command == 'x')
    {
      //Do nothing, just exit
      Serial.println(F("Exiting"));
      return;
    }

  }
}

//Configure how many readings to average together
void average_reading_setup(void)
{
  Serial.println(F("Number of readings to average together"));
  Serial.println(F("Press + or a to increase"));
  Serial.println(F("Press - or z to decrease"));
  Serial.println(F("Press x to exit"));

  Serial.print(F("Number of readings: "));
  Serial.println(setting_average_amount);

  while(1)
  {
    if(Serial.available())
    {
      char temp = Serial.read();
      
      if(temp == '+' || temp == 'a')
      {
        if(setting_average_amount < 64) setting_average_amount++;
      }
      else if(temp == '-' || temp == 'z')
      {
        if(setting_average_amount > 1) setting_average_amount--;
      }
      else if(temp == 'x')
        break;

      Serial.print(F("Number of readings: "));
      Serial.println(setting_average_amount);
    }
  }

  //Record this new value to EEPROM
  record_system_settings();
}

//Configure how many decimals to show
void decimal_setup(void)
{
  Serial.println(F("Press + or a to increase number of decimals"));
  Serial.println(F("Press - or z to decrease number of decimals"));
  Serial.println(F("Press x to exit"));

  Serial.print(F("Decimal places: "));
  Serial.println(setting_decimal_places);

  while(1)
  {
    if(Serial.available())
    {
      char temp = Serial.read();
      
      if(temp == '+' || temp == 'a')
      {
        if(setting_decimal_places < 4) setting_decimal_places++;
      }
      else if(temp == '-' || temp == 'z')
      {
        if(setting_decimal_places > 0) setting_decimal_places--;
      }
      else if(temp == 'x')
        break;

      Serial.print(F("Decimal places: "));
      Serial.println(setting_decimal_places);
    }
  }

  //Record this new value to EEPROM
  record_system_settings();
}

//Configure what baud rate to communicate at
void baud_setup(void)
{
  Serial.print(F("\n\rCurrent rate: "));
  Serial.print(setting_uart_speed, DEC);
  Serial.println(F(" bps"));

  Serial.println(F("Enter new baud rate ('x' to exit):"));

  //Print prompt
  Serial.print(F(">"));

  //Read user input
  char newBaud[8]; //Max at 1000000
  read_line(newBaud, sizeof(newBaud));

  //Look for escape character
  if(newBaud[0] == 'x')
  {
    Serial.println(F("Exiting"));
    return;
  }

  long newRate = strtolong(newBaud); //Convert this string to a long

  if(newRate < BAUD_MIN || newRate > BAUD_MAX)
  {
    Serial.println(F("Out of bounds"));
  }
  else
  {
    Serial.print(F("Going to "));
    Serial.print(newRate);
    Serial.println(F("bps"));

    //Record this new baud rate
    setting_uart_speed = newRate;
    record_system_settings();

    //Go to this speed
    Serial.end();
    Serial.begin(setting_uart_speed);
  }
}

//Determine how much time we need between measurements
//Takes into account current baud rate
int calcMinimumReadTime(void)
{
  //The first few reads take too little time
  scale.get_units();
  scale.get_units();

  //Now do an average
  long startTime = millis();
  for(byte x = 0 ; x < 8 ; x++)
    scale.get_units(); //Do a dummy read and time it
  int averageReadTime = (millis() - startTime) / 8;
  
  averageReadTime *= setting_average_amount; //Increase this time by the number of system reads we want to do

  //Assume we will need to print a minimum of 7 characters at this baud rate per loop
  //1 / 9600 = 1ms * 10bits per byte = 9.6ms per byte
  float characterTime = 10000 / (float)setting_uart_speed;

  //Calculate number of characters per report
  //"51588595,178.899,lbs"
  int characters = strlen("123.,"); //Basic case

  characters += setting_decimal_places; //For example this would pickup 3 of '17.123'

  if(setting_units == UNITS_LBS) characters += strlen("lbs");
  if(setting_units == UNITS_KG) characters += strlen("kg");

  if(setting_time_stamp == true) characters += strlen("51588595,"); //14.33 hours

  //Serial.print("characterTime: ");
  //Serial.println(ceil((float)characters * characterTime));
  
  return(averageReadTime + ceil((float)characters * characterTime)); 
}

//Gives user the ability to set a known weight on the scale and calculate a calibration factor
void calibrate_scale(void)
{
  Serial.println(F("Scale calibration"));
  Serial.println(F("Remove all weight from scale"));
  Serial.println(F("After readings begin, place known weight on scale"));
  Serial.println(F("Press + or a to increase calibration factor"));
  Serial.println(F("Press - or z to decrease calibration factor"));
  Serial.println(F("Press x to exit"));

  delay(3000); //Delay so user can read instructions

  long lastChange = millis();
  int changeRate = 1;
  int holdDownCounter = 0;

  while(1)
  {
    scale.set_scale(setting_calibration_factor); //Adjust to this calibration factor

    Serial.print(F("Reading: ["));
    Serial.print(scale.get_units(setting_average_amount), 4); //Show 4 decimals during calibration
    //Serial.print(scale.get_units(setting_average_amount), setting_decimal_places);
    Serial.print(F(" "));
    if(setting_units == UNITS_LBS) Serial.print(F("lbs"));
    if(setting_units == UNITS_KG) Serial.print(F("kg"));
    Serial.print(F("]   Calibration Factor: "));
    Serial.print(setting_calibration_factor);
    Serial.println();

    if(Serial.available())
    {
      //toggledLED(); //Blink serial indicator

      //Check to see if user is holding down the button
      long delta = millis() - lastChange;
      lastChange = millis();
      
      if(delta > 100) //Slow, just 1 and reset holdDown counter
      {
        changeRate = 1;
        holdDownCounter = 0;
      }
      else //Medium 10 and increment counter
      {
        changeRate = 10;
        holdDownCounter++;
        if(holdDownCounter > 60)
        {
          holdDownCounter = 100; //Don't let this get too big
          changeRate = 100; //Change faster
        }
        else if(holdDownCounter > 25)
        {
          changeRate = 25; //Change faster
        }
      }

      while(Serial.available())
      {
        char temp = Serial.read();
  
        if(temp == '+' || temp == 'a')
          setting_calibration_factor += changeRate;
        else if(temp == '-' || temp == 'z')
          setting_calibration_factor -= changeRate;
        else if(temp == 'x')
        {
          //Record this new value to EEPROM
          record_system_settings();
          return;
        }
      }
    }
  }

}

//Allow user to input the time between readings
void rate_setup(void)
{
  //Calculate the minimum time between reports
  int minTime = calcMinimumReadTime();
  
  Serial.println(F("Press + or a to increase time between reports"));
  Serial.println(F("Press - or z to decrease time between reports"));
  Serial.println(F("Press x to exit"));

  Serial.print(F("Minimum: "));
  Serial.print(minTime);
  Serial.println("ms");
  
  if(setting_report_rate < minTime) setting_report_rate = minTime;

  Serial.print(F("Time: "));
  Serial.print(setting_report_rate);
  Serial.println("ms");

  long lastChange = millis();
  int changeRate = 1;
  int holdDownCounter = 0;
  
  while(1)
  {
    if(Serial.available())
    {
      char temp = Serial.read();
      
      //Check to see if user is holding down the button
      long delta = millis() - lastChange;
      lastChange = millis();
      
      if(delta > 100) //Slow, just change one at a time and reset holdDown counter
      {
        changeRate = 1;
        holdDownCounter = 0;
      }
      else //You're holding the button down, change 10, 25, or500300 medium 10 and increment counter
      {
        changeRate = 10;
        holdDownCounter++;
        if(holdDownCounter > 500)
        {
          holdDownCounter = 500; //Don't let this get too big
          changeRate = 500; //Change faster
        }
        else if(holdDownCounter > 25)
        {
          changeRate = 25; //Change faster
        }
      }

      if(temp == '+' || temp == 'a')
        setting_report_rate += changeRate;
      else if(temp == '-' || temp == 'z')
      {
        if(changeRate > setting_report_rate) //Catch a case where we could go negative
          setting_report_rate = 0;
        else
          setting_report_rate -= changeRate;
      }
      else if(temp == 'x')
        break;

      if(setting_report_rate < minTime) setting_report_rate = minTime;
      if(setting_report_rate > 50000) setting_report_rate = 50000; //Max of 50 seconds

      Serial.print(F("Time: "));
      Serial.print(setting_report_rate);
      Serial.println("ms");
    }
  }

  //Record this new value to EEPROM
  record_system_settings();
}

//Toggle the status LED
void toggleLED()
{
  if(digitalRead(statusLED))
    digitalWrite(statusLED, LOW);
  else
    digitalWrite(statusLED, HIGH);
}

//Check to see if we need an emergency UART reset
//Scan the RX pin for 2 seconds
//If it's low the entire time, then return 1
void check_emergency_reset(void)
{
  pinMode(0, INPUT); //Turn the RX pin into an input
  digitalWrite(0, HIGH); //Push a 1 onto RX pin to enable internal pull-up

  //Quick pin check
  if(digitalRead(0) == HIGH) return;

  Serial.println("Reset!");

  //Wait 2 seconds, blinking LED while we wait
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, LOW); //Set the STAT1 LED

  for(byte i = 0 ; i < 80 ; i++)
  {
    delay(25);

    toggleLED();

    if(digitalRead(0) == HIGH) return; //Check to see if RX is not low anymore
  }		

  //If we make it here, then RX pin stayed low the whole time
  set_default_settings(); //Reset baud, escape characters, escape number, system mode

  //Now sit in forever loop indicating system is now at 9600bps
  while(1)
  {
    delay(500);
    toggleLED();
  }
}

//Resets all the system settings to safe values
void set_default_settings(void)
{
  //Reset UART to 9600bps
  setting_uart_speed = 9600;

  //Reset to pounds as our unit of measure
  setting_units = UNITS_LBS;

  //Reset report rate to 5Hz
  setting_report_rate = 200;

  //Reset calibration factor
  setting_calibration_factor = 1000;

  //Reset tare point
  setting_tare_point = 0;

  //Reset time stamp
  setting_time_stamp = true;
  
  //Reset decimals
  setting_decimal_places = 2;
  
  //Reset average amount
  setting_average_amount = 4;

  //Commit these new settings to memory  
  record_system_settings();
}

//Record the current system settings to EEPROM
void record_system_settings(void)
{
  writeBytes(LOCATION_BAUD_MSB, setting_uart_speed, sizeof(setting_uart_speed));

  EEPROM.write(LOCATION_MASS_UNITS, setting_units);

  writeBytes(LOCATION_REPORT_RATE_MSB, setting_report_rate, sizeof(setting_report_rate));

  writeBytes(LOCATION_CALIBRATION_FACTOR_MSB, setting_calibration_factor, sizeof(setting_calibration_factor));

  writeBytes(LOCATION_TARE_POINT_MSB, setting_tare_point, sizeof(setting_tare_point));

  EEPROM.write(LOCATION_TIME_STAMP, setting_time_stamp);

  EEPROM.write(LOCATION_DECIMAL_PLACES, setting_decimal_places);

  EEPROM.write(LOCATION_AVERAGE_AMOUNT, setting_average_amount);
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value
void read_system_settings(void)
{
  //Read what the current UART speed is from EEPROM memory
  setting_uart_speed = readBytes(LOCATION_BAUD_MSB, sizeof(setting_uart_speed));

  if(setting_uart_speed < BAUD_MIN || setting_uart_speed > BAUD_MAX) 
  {
    setting_uart_speed = 9600; //Reset UART to 9600 if there is no speed stored
    writeBytes(LOCATION_BAUD_MSB, setting_uart_speed, sizeof(setting_uart_speed));
  }

  //Determine the units we should be using
  setting_units = EEPROM.read(LOCATION_MASS_UNITS);
  Serial.begin(9600);
  Serial.print("Units: ");
  Serial.println(setting_units);
  
  if(setting_units > 1)
  {
    setting_units = UNITS_LBS; //Default to lbs
    EEPROM.write(LOCATION_MASS_UNITS, setting_units);
  }

  //Determine the report rate
  setting_report_rate = readBytes(LOCATION_REPORT_RATE_MSB, sizeof(setting_report_rate));
  if(setting_report_rate == 0xFFFF) 
  {
    setting_report_rate = 200; //Default to 200ms
    writeBytes(LOCATION_REPORT_RATE_MSB, setting_report_rate, sizeof(setting_report_rate));
  }

  //Look up the calibration factor
  setting_calibration_factor = readBytes(LOCATION_CALIBRATION_FACTOR_MSB, sizeof(setting_calibration_factor));
  if(setting_calibration_factor == 0xFFFFFFFF) 
  {
    setting_calibration_factor = 0; //Default to 0
    writeBytes(LOCATION_CALIBRATION_FACTOR_MSB, setting_calibration_factor, sizeof(setting_calibration_factor));
  }

  //Look up the zero tare point
  setting_tare_point = readBytes(LOCATION_TARE_POINT_MSB, sizeof(setting_tare_point));
  if(setting_tare_point == 0xFFFFFFFF) 
  {
    setting_tare_point = 1000; //Default to 1000 so we don't get inf
    writeBytes(LOCATION_TARE_POINT_MSB, setting_tare_point, sizeof(setting_tare_point));
  }

  //Determine if we need time stamps
  setting_time_stamp = EEPROM.read(LOCATION_TIME_STAMP);
  if(setting_time_stamp > 2) 
  {
    setting_time_stamp = true; //Default to true
    EEPROM.write(LOCATION_TIME_STAMP, setting_time_stamp);
  }

  //Look up decimals
  setting_decimal_places = EEPROM.read(LOCATION_DECIMAL_PLACES);
  if(setting_decimal_places > 5) 
  {
    setting_decimal_places = 2; //Default to 2
    EEPROM.write(LOCATION_DECIMAL_PLACES, setting_decimal_places);
  }

  //Look up average amount
  setting_average_amount = EEPROM.read(LOCATION_AVERAGE_AMOUNT);
  if(setting_average_amount > 64 || setting_average_amount == 0) 
  {
    setting_average_amount = 4; //Default to 4
    EEPROM.write(LOCATION_AVERAGE_AMOUNT, setting_average_amount);
  }
}

//Record a series of bytes to EEPROM starting at address
void writeBytes(byte address, long setting, byte sizeOfSetting)
{
  for(byte x = 0 ; x < sizeOfSetting ; x++)
  {
    byte toWrite = setting >> 8 * (sizeOfSetting - 1 - x);
    EEPROM.write(address + x, toWrite);   
  }
}

//Give a location read out a number of bytes
long readBytes(byte address, byte sizeOfSetting)
{
  long setting = 0;

  for(byte x = 0 ; x < sizeOfSetting ; x++)
  {
    setting <<= 8;
    setting |= EEPROM.read(address + x);
  }

  return(setting);
}

//Reads a line until the \n enter character is found
byte read_line(char* buffer, byte buffer_length)
{
  memset(buffer, 0, buffer_length); //Clear buffer

  byte read_length = 0;
  while(read_length < buffer_length - 1) {
    while (!Serial.available());
    byte c = Serial.read();

    toggleLED(); //Blink status LED with each character received
    
    if(c == 0x08 || c == 0x7f) { //Backspace characters
      if(read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0'; //Put a terminator on the string in case we are finished

      Serial.print((char)0x08); //Move back one space
      Serial.print(F(" ")); //Put a blank there to erase the letter from the terminal
      Serial.print((char)0x08); //Move back again

      continue;
    }

    Serial.print((char)c); //Echo the user's input

    if(c == '\r') {
      Serial.println();
      buffer[read_length] = '\0';
      break;
    }
    else if (c == '\n') {
    }
    else {
      buffer[read_length] = c;
      ++read_length;
    }
  }

  return read_length;
}

//A rudimentary way to convert a string to a long 32 bit integer
//Used by the read command, in command shell and baud from the system menu
uint32_t strtolong(const char* str)
{
  uint32_t l = 0;
  while(*str >= '0' && *str <= '9')
    l = l * 10 + (*str++ - '0');

  return l;
}
