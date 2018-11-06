/*
 These are lower level system functions
*/

//Check to see if we need an emergency UART reset
//Scan the RX pin for 2 seconds
//If it's low the entire time, then return 1
void checkEmergencyReset(void)
{
  pinMode(0, INPUT); //Turn the RX pin into an input
  digitalWrite(0, HIGH); //Push a 1 onto RX pin to enable internal pull-up

  //Quick pin check
  if (digitalRead(0) == HIGH) return;

  Serial.println(F("Reset!"));

  //Wait 2 seconds, blinking LED while we wait
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, LOW); //Set the STAT1 LED

  for (byte i = 0 ; i < 80 ; i++)
  {
    delay(25);

    toggleLED();

    if (digitalRead(0) == HIGH) return; //Check to see if RX is not low anymore
  }

  //If we make it here, then RX pin stayed low the whole time
  set_default_settings(); //Reset baud, escape characters, escape number, system mode

  //Now sit in forever loop indicating system is now at 9600bps
  while (1)
  {
    delay(1000);
    toggleLED();
    Serial.println(F("Reset - please power cycle"));
  }
}

void powerUpScale(void)
{
  scale.power_up();
}

void powerDownScale(void)
{
  scale.power_down();
}

//Toggle the status LED
void toggleLED()
{
  if (digitalRead(statusLED))
    digitalWrite(statusLED, LOW);
  else
    digitalWrite(statusLED, HIGH);
}

//Resets all the system settings to safe values
void set_default_settings(void)
{
  //Reset UART to 9600bps
  setting_uart_speed = 9600;

  //Reset to pounds as our unit of measure
  setting_units = UNITS_LBS;

  //Reset report rate to 2Hz
  setting_report_rate = 500;

  //Reset calibration factor
  setting_calibration_factor = 1000;

  //Reset tare point
  setting_tare_point = 0;

  //Reset time stamp
  setting_timestamp_enable = true;

  //Reset decimals
  setting_decimal_places = 2;

  //Reset average amount
  setting_average_amount = 4;

  //Reset local temp
  setting_local_temp_enable = true;

  //Reset remote temp
  setting_remote_temp_enable = true;

  //Reset LED blinking
  setting_status_enable = true;

  //Reset serial trigger
  setting_serial_trigger_enable = false;

  //Reset raw reading
  setting_raw_reading_enable = false;

  //Reset trigger character
  setting_trigger_character = '!';

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

  EEPROM.write(LOCATION_TIMESTAMP_ENABLE, setting_timestamp_enable);

  EEPROM.write(LOCATION_DECIMAL_PLACES, setting_decimal_places);

  EEPROM.write(LOCATION_AVERAGE_AMOUNT, setting_average_amount);

  EEPROM.write(LOCATION_LOCAL_TEMP_ENABLE, setting_local_temp_enable);

  EEPROM.write(LOCATION_REMOTE_TEMP_ENABLE, setting_remote_temp_enable);

  EEPROM.write(LOCATION_STATUS_ENABLE, setting_status_enable);

  EEPROM.write(LOCATION_SERIAL_TRIGGER_ENABLE, setting_serial_trigger_enable);

  EEPROM.write(LOCATION_RAW_READING_ENABLE, setting_raw_reading_enable);

  EEPROM.write(LOCATION_TRIGGER_CHARACTER, setting_trigger_character);
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value
void readSystemSettings(void)
{
  //Read what the current UART speed is from EEPROM memory
  setting_uart_speed = readBytes(LOCATION_BAUD_MSB, sizeof(setting_uart_speed));
  if (setting_uart_speed < BAUD_MIN || setting_uart_speed > BAUD_MAX)
  {
    setting_uart_speed = 9600; //Reset UART to 9600 if there is no speed stored
    writeBytes(LOCATION_BAUD_MSB, setting_uart_speed, sizeof(setting_uart_speed));
  }

  //Determine the units we should be using
  setting_units = EEPROM.read(LOCATION_MASS_UNITS);
  if (setting_units > 1)
  {
    setting_units = UNITS_LBS; //Default to lbs
    EEPROM.write(LOCATION_MASS_UNITS, setting_units);
  }

  //Determine the report rate
  setting_report_rate = readBytes(LOCATION_REPORT_RATE_MSB, sizeof(setting_report_rate));
  if (setting_report_rate == 0xFFFF)
  {
    setting_report_rate = 200; //Default to 200ms
    writeBytes(LOCATION_REPORT_RATE_MSB, setting_report_rate, sizeof(setting_report_rate));
  }

  //Look up the calibration factor
  setting_calibration_factor = readBytes(LOCATION_CALIBRATION_FACTOR_MSB, sizeof(setting_calibration_factor));
  if (setting_calibration_factor == 0xFFFFFFFF)
  {
    setting_calibration_factor = 0; //Default to 0
    writeBytes(LOCATION_CALIBRATION_FACTOR_MSB, setting_calibration_factor, sizeof(setting_calibration_factor));
  }

  //Look up the zero tare point
  setting_tare_point = readBytes(LOCATION_TARE_POINT_MSB, sizeof(setting_tare_point));
  if (setting_tare_point == 0xFFFFFFFF)
  {
    setting_tare_point = 1000; //Default to 1000 so we don't get inf
    writeBytes(LOCATION_TARE_POINT_MSB, setting_tare_point, sizeof(setting_tare_point));
  }

  //Determine if we need time stamps
  setting_timestamp_enable = EEPROM.read(LOCATION_TIMESTAMP_ENABLE);
  if (setting_timestamp_enable > 2)
  {
    setting_timestamp_enable = true; //Default to true
    EEPROM.write(LOCATION_TIMESTAMP_ENABLE, setting_timestamp_enable);
  }

  //Look up decimals
  setting_decimal_places = EEPROM.read(LOCATION_DECIMAL_PLACES);
  if (setting_decimal_places > 5)
  {
    setting_decimal_places = 2; //Default to 2
    EEPROM.write(LOCATION_DECIMAL_PLACES, setting_decimal_places);
  }

  //Look up average amount
  setting_average_amount = EEPROM.read(LOCATION_AVERAGE_AMOUNT);
  if (setting_average_amount > 64 || setting_average_amount == 0)
  {
    setting_average_amount = 4; //Default to 4
    EEPROM.write(LOCATION_AVERAGE_AMOUNT, setting_average_amount);
  }

  //Look up if we are reporting local temperature or not
  setting_local_temp_enable = EEPROM.read(LOCATION_LOCAL_TEMP_ENABLE);
  if (setting_local_temp_enable > 1)
  {
    setting_local_temp_enable = true; //Default to true
    EEPROM.write(LOCATION_LOCAL_TEMP_ENABLE, setting_local_temp_enable);
  }

  //Look up if we are reporting remote temperature or not
  setting_remote_temp_enable = EEPROM.read(LOCATION_REMOTE_TEMP_ENABLE);
  if (setting_remote_temp_enable > 1)
  {
    setting_remote_temp_enable = true; //Default to true
    EEPROM.write(LOCATION_REMOTE_TEMP_ENABLE, setting_remote_temp_enable);
  }

  //Look up if we are blinking the status LED
  setting_status_enable = EEPROM.read(LOCATION_STATUS_ENABLE);
  if (setting_status_enable > 1)
  {
    setting_status_enable = true; //Default to true
    EEPROM.write(LOCATION_STATUS_ENABLE, setting_status_enable);
  }

  //Look up if we do a reading when a serial character is received
  setting_serial_trigger_enable = EEPROM.read(LOCATION_SERIAL_TRIGGER_ENABLE);
  if (setting_serial_trigger_enable > 1)
  {
    setting_serial_trigger_enable = false; //Default to false
    EEPROM.write(LOCATION_SERIAL_TRIGGER_ENABLE, setting_serial_trigger_enable);
  }

  //Look up if we need to output the raw reading
  setting_raw_reading_enable = EEPROM.read(LOCATION_RAW_READING_ENABLE);
  if (setting_raw_reading_enable > 1)
  {
    setting_raw_reading_enable = false; //Default to false
    EEPROM.write(LOCATION_RAW_READING_ENABLE, setting_raw_reading_enable);
  }

  //Look up the character to trigger a reading
  setting_trigger_character = EEPROM.read(LOCATION_TRIGGER_CHARACTER);
  if (setting_trigger_character == 255)
  {
    setting_trigger_character = '!'; //Default to !
    EEPROM.write(LOCATION_TRIGGER_CHARACTER, setting_trigger_character);
  }

}

//Record a series of bytes to EEPROM starting at address
void writeBytes(byte address, long setting, byte sizeOfSetting)
{
  for (byte x = 0 ; x < sizeOfSetting ; x++)
  {
    byte toWrite = setting >> 8 * (sizeOfSetting - 1 - x);
    EEPROM.write(address + x, toWrite);
  }
}

//Give a location read out a number of bytes
long readBytes(byte address, byte sizeOfSetting)
{
  long setting = 0;

  for (byte x = 0 ; x < sizeOfSetting ; x++)
  {
    setting <<= 8;
    setting |= EEPROM.read(address + x);
  }

  return (setting);
}


//Reads a line until the \n enter character is found
byte read_line(char* buffer, byte buffer_length)
{
  memset(buffer, 0, buffer_length); //Clear buffer

  byte read_length = 0;
  while (read_length < buffer_length - 1) {
    while (!Serial.available());
    byte c = Serial.read();

    toggleLED(); //Blink status LED with each character received

    if (c == 0x08 || c == 0x7f) { //Backspace characters
      if (read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0'; //Put a terminator on the string in case we are finished

      Serial.print((char)0x08); //Move back one space
      Serial.print(F(" ")); //Put a blank there to erase the letter from the terminal
      Serial.print((char)0x08); //Move back again

      continue;
    }

    Serial.print((char)c); //Echo the user's input

    if (c == '\r') {
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
  while (*str >= '0' && *str <= '9')
    l = l * 10 + (*str++ - '0');

  return l;
}
