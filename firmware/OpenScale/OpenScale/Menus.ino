/*
 Lots of serial menus and visual stuff so user can configure the OpenScale
*/

//Configure how OpenScale operates
void system_setup(void)
{
  while (1)
  {
    displaySystemHeader(); //Product title and firmware version

    Serial.println(F("System Configuration"));

    Serial.print(F("1) Tare scale to zero ["));
    Serial.print(setting_tare_point);
    Serial.println(F("]"));

    Serial.print(F("2) Calibrate scale ["));
    Serial.print(setting_calibration_factor);
    Serial.println(F("]"));

    Serial.print(F("3) Timestamp [O"));
    if (setting_timestamp_enable == true) Serial.print(F("n"));
    else Serial.print(F("ff"));
    Serial.println(F("]"));

    Serial.print(F("4) Set report rate ["));
    Serial.print(setting_report_rate);
    Serial.println(F("]"));

    Serial.print(F("5) Set baud rate ["));
    Serial.print(setting_uart_speed);
    Serial.println(F(" bps]"));

    Serial.print(F("6) Change units of measure ["));
    if (setting_units == UNITS_KG) Serial.print(F("kg"));
    if (setting_units == UNITS_LBS) Serial.print(F("lbs"));
    Serial.println(F("]"));

    Serial.print(F("7) Decimals ["));
    Serial.print(setting_decimal_places);
    Serial.println(F("]"));

    Serial.print(F("8) Average amount ["));
    Serial.print(setting_average_amount);
    Serial.println(F("]"));

    Serial.print(F("9) Local temp [O"));
    if (setting_local_temp_enable == true) Serial.print(F("n"));
    else Serial.print(F("ff"));
    Serial.println(F("]"));

    Serial.print(F("r) Remote temp [O"));
    if (setting_remote_temp_enable == true) Serial.print(F("n"));
    else Serial.print(F("ff"));
    Serial.println(F("]"));

    Serial.print(F("s) Status LED ["));
    if (setting_status_enable == true) Serial.print(F("Blink"));
    else Serial.print(F("Off"));
    Serial.println(F("]"));

    Serial.print(F("t) Serial trigger [O"));
    if (setting_serial_trigger_enable == true) Serial.print(F("n"));
    else Serial.print(F("ff"));
    Serial.println(F("]"));

    Serial.print(F("q) Raw reading [O"));
    if (setting_raw_reading_enable == true) Serial.print(F("n"));
    else Serial.print(F("ff"));
    Serial.println(F("]"));

    Serial.print(F("c) Trigger character: ["));
    Serial.print(setting_trigger_character);
    Serial.println(F("]"));


    Serial.println(F("x) Exit"));
    Serial.print(F(">"));

    //Read command
    while (!Serial.available()) ; //Wait for user to type a character
    char command = Serial.read();

    toggleLED();

    //Execute command
    if (command == '1')
    {
      //This can be used to remove the need to tare the scale.
      //Useful in permanent scale projects.

      scale.tare(); //Reset the scale to 0


      setting_tare_point = scale.read_average(10); //Get 10 readings from the HX711 and average them
      Serial.print(F("\n\rTare point 1: "));
      Serial.println(setting_tare_point);

      //Try a different method
      setting_tare_point = 0;
      for (int x = 0 ; x < 10 ; x++)
      {
        setting_tare_point += scale.read(); //Get a reading
        delay(100);
      }
      setting_tare_point /= 10;

      Serial.print(F("\n\rTare point 2: "));
      Serial.println(setting_tare_point);

      record_system_settings();
    }
    else if (command == '2')
    {
      calibrate_scale();
    }
    else if (command == '3')
    {
      Serial.print(F("\n\rTimestamp o"));
      if (setting_timestamp_enable == true)
      {
        Serial.println(F("ff"));
        setting_timestamp_enable = false;
      }
      else
      {
        Serial.println(F("n"));
        setting_timestamp_enable = true;
      }
      record_system_settings();
    }
    else if (command == '4')
    {
      rate_setup();
    }
    else if (command == '5')
    {
      baud_setup();
    }
    else if (command == '6')
    {
      Serial.print(F("\n\rUnits set to "));
      if (setting_units == UNITS_KG)
      {
        Serial.println(F("lbs"));
        setting_units = UNITS_LBS;
        float newFactor = (float)setting_calibration_factor * 0.453592; //Convert the calibration factor from kg to lbs
        setting_calibration_factor = (long)newFactor;
      }
      else if (setting_units == UNITS_LBS)
      {
        Serial.println(F("kg"));
        setting_units = UNITS_KG;
        float newFactor = (float)setting_calibration_factor * 2.20462; //Convert the calibration factor from lbs to kg
        setting_calibration_factor = (long)newFactor;
      }

      scale.set_scale(setting_calibration_factor); //Assign this new factor to the scale
      record_system_settings();

    }
    else if (command == '7')
    {
      decimal_setup();
    }
    else if (command == '8')
    {
      average_reading_setup();
    }
    else if (command == '9')
    {
      Serial.print(F("\n\rLocal temp o"));
      if (setting_local_temp_enable == true)
      {
        Serial.println(F("ff"));
        setting_local_temp_enable = false;
      }
      else
      {
        Serial.println(F("n"));
        setting_local_temp_enable = true;
      }
      record_system_settings();
    }
    else if (command == 'r')
    {
      Serial.print(F("\n\rRemote temp o"));
      if (setting_remote_temp_enable == true)
      {
        Serial.println(F("ff"));
        setting_remote_temp_enable = false;
      }
      else
      {
        Serial.println(F("n"));
        setting_remote_temp_enable = true;
      }
      record_system_settings();
    }

    else if (command == 's')
    {
      Serial.print(F("\n\rStatus LED o"));
      if (setting_status_enable == true)
      {
        Serial.println(F("ff"));
        setting_status_enable = false;
        digitalWrite(statusLED, LOW); //Turn off the LED
      }
      else
      {
        Serial.println(F("n"));
        setting_status_enable = true;
      }
      record_system_settings();
    }
    else if (command == 't')
    {
      Serial.print(F("\n\rSerial trigger o"));
      if (setting_serial_trigger_enable == true)
      {
        Serial.println(F("ff"));
        setting_serial_trigger_enable = false;
      }
      else
      {
        Serial.println(F("n"));
        setting_serial_trigger_enable = true;
      }
      record_system_settings();
    }
    else if (command == 'q')
    {
      Serial.print(F("\n\rRaw reading o"));
      if (setting_raw_reading_enable == true)
      {
        Serial.println(F("ff"));
        setting_raw_reading_enable = false;
      }
      else
      {
        Serial.println(F("n"));
        setting_raw_reading_enable = true;
      }
      record_system_settings();
    }
    else if (command == 'c')
    {
      Serial.print(F("\n\rEnter new trigger character: "));

      while (Serial.available() == false) delay(1);

      setting_trigger_character = Serial.read();

      Serial.println();
      Serial.print(F("\n\rNew character: "));
      Serial.print(setting_trigger_character);

      record_system_settings();
    }
    else if (command == 'x')
    {
      //Do nothing, just exit
      Serial.println(F("Exiting"));
      return;
    }

  }
}

//Gives user the ability to set a known weight on the scale and calculate a calibration factor
void calibrate_scale(void)
{
  Serial.println();
  Serial.println();
  Serial.println(F("Scale calibration"));
  Serial.println(F("Place known weight on scale. Press a key when weight is in place and stable."));

  while(Serial.available() == false) ; //Wait for user to press key

  Serial.print(F("Tare: "));
  Serial.println(setting_tare_point);

  long rawReading = scale.read_average(setting_average_amount); //Take average reading over a given number of times
  Serial.print(F("Raw: "));
  Serial.println(rawReading);

  Serial.print(F("Current Reading: "));
  Serial.print(scale.get_units(setting_average_amount), 4); //Show 4 decimals during calibration
  if (setting_units == UNITS_LBS) Serial.print(F("lbs"));
  if (setting_units == UNITS_KG) Serial.print(F("kg"));
  Serial.println();

  Serial.print(F("Calibration Factor: "));
  Serial.print(setting_calibration_factor);
  Serial.println();

  while(Serial.available()) Serial.read(); //Clear anything in RX buffer

  Serial.print(F("Please enter the weight currently sitting on the scale: "));

  while (Serial.available() == false) ; //Wait for user input

  Serial.setTimeout(1000); //Wait for user to press return or stop typing for 1 second
  float weightOnScale = Serial.parseFloat();
  //TODO create function here to echo user's typing and allow for backspaces
  Serial.println();

  while(Serial.available()) Serial.read(); //Clear anything in RX buffer

  Serial.print(F("User entered: "));
  Serial.println(weightOnScale, 4);

  //Convert this weight to a calibration factor

  //tare: 210193
  //raw: 246177
  //User Input: 0.5276 kg
  //avg: 4 times

  //get_units = (raw-OFFSET) / calibration_factor
  //0.5276 = (246177-210193) / cal_factor
  //114185 / .45 = 256744

  setting_calibration_factor = (rawReading - setting_tare_point) / weightOnScale;

  Serial.print(F("New Calibration Factor: "));
  Serial.print(setting_calibration_factor);
  Serial.println();

  scale.set_scale(setting_calibration_factor); //Go to this new cal factor

  //Record this new value to EEPROM
  record_system_settings();

  Serial.print(F("New Scale Reading: "));
  Serial.print(scale.get_units(setting_average_amount), 4); //Show 4 decimals during calibration
  Serial.print(F(" "));
  if (setting_units == UNITS_LBS) Serial.print(F("lbs"));
  if (setting_units == UNITS_KG) Serial.print(F("kg"));
  Serial.println();
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

  while (1)
  {
    if (Serial.available())
    {
      char temp = Serial.read();

      if (temp == '+' || temp == 'a')
      {
        if (setting_average_amount < 64) setting_average_amount++;
      }
      else if (temp == '-' || temp == 'z')
      {
        if (setting_average_amount > 1) setting_average_amount--;
      }
      else if (temp == 'x')
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

  while (1)
  {
    if (Serial.available())
    {
      char temp = Serial.read();

      if (temp == '+' || temp == 'a')
      {
        if (setting_decimal_places < 4) setting_decimal_places++;
      }
      else if (temp == '-' || temp == 'z')
      {
        if (setting_decimal_places > 0) setting_decimal_places--;
      }
      else if (temp == 'x')
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

  Serial.println(F("Enter new baud rate ('x' to abort):"));

  //Print prompt
  Serial.print(F(">"));

  //Read user input
  char newBaud[8]; //Max at 1000000
  read_line(newBaud, sizeof(newBaud));

  //Look for escape character
  if (newBaud[0] == 'x')
  {
    Serial.println(F("Exiting"));
    return;
  }

  long newRate = strtolong(newBaud); //Convert this string to a long

  if (newRate < BAUD_MIN || newRate > BAUD_MAX)
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
  Serial.println(F("ms"));

  if (setting_report_rate < minTime) setting_report_rate = minTime;

  Serial.print(F("Time: "));
  Serial.print(setting_report_rate);
  Serial.println(F("ms"));

  long lastChange = millis();
  int changeRate = 1;
  int holdDownCounter = 0;

  while (1)
  {
    if (Serial.available())
    {
      char temp = Serial.read();

      //Check to see if user is holding down the button
      long delta = millis() - lastChange;
      lastChange = millis();

      if (delta > 100) //Slow, just change one at a time and reset holdDown counter
      {
        changeRate = 1;
        holdDownCounter = 0;
      }
      else //You're holding the button down, change 10, 25, or500300 medium 10 and increment counter
      {
        changeRate = 10;
        holdDownCounter++;
        if (holdDownCounter > 500)
        {
          holdDownCounter = 500; //Don't let this get too big
          changeRate = 500; //Change faster
        }
        else if (holdDownCounter > 25)
        {
          changeRate = 25; //Change faster
        }
      }

      if (temp == '+' || temp == 'a')
        setting_report_rate += changeRate;
      else if (temp == '-' || temp == 'z')
      {
        if (changeRate > setting_report_rate) //Catch a case where we could go negative
          setting_report_rate = 0;
        else
          setting_report_rate -= changeRate;
      }
      else if (temp == 'x')
        break;

      if (setting_report_rate < minTime) setting_report_rate = minTime;
      if (setting_report_rate > 50000) setting_report_rate = 50000; //Max of 50 seconds

      Serial.print(F("Time: "));
      Serial.print(setting_report_rate);
      Serial.println(F("ms"));
    }
  }

  //Record this new value to EEPROM
  record_system_settings();
}

//Determine how much time we need between measurements
//Takes into account current baud rate
//Takes into account the time to read various sensors
//Takes into account raw reading printing
int calcMinimumReadTime(void)
{
  //The first few reads take too little time
  scale.get_units();
  scale.get_units();

  //Establish out much time it takes to do a standard scale read
  long startTime = millis();
  scale.get_units(setting_average_amount); //Do a dummy read and time it
  int averageReadTime = ceil((millis() - startTime));
  int sensorReadTime = averageReadTime;

  //Assume we will need to print a minimum of 7 characters at this baud rate per loop
  //1 / 9600 = 1ms * 10bits per byte = 9.6ms per byte
  float characterTime = 10000 / (float)setting_uart_speed;

  //Calculate number of characters per report
  int characters = 0;

  if (setting_timestamp_enable == true) characters += strlen("51588595,"); //Timestamp has characters

  if (setting_local_temp_enable)
  {
    //Establish how much time it takes to do a local temp read
    long startTime = millis();
    for (byte x = 0 ; x < 8 ; x++)
      getLocalTemperature(); //Do a dummy read and time it
    averageReadTime = ceil((millis() - startTime) / (float)8);
    sensorReadTime += averageReadTime; //In ms

    characters += strlen("24.75,"); //Add the time it takes to print the characters as well
  }

  if (setting_remote_temp_enable)
  {
    //Establish how much time it takes to do a remote temp read
    long startTime = millis();
    for (byte x = 0 ; x < 8 ; x++)
      getRemoteTemperature(); //Do a dummy read and time it
    averageReadTime = ceil((millis() - startTime) / (float)8);
    sensorReadTime += averageReadTime; //In ms

    characters += strlen("27.81,"); //Add the time it takes to print the characters as well
  }

  characters += strlen("123,"); //Basic weight without decimals

  if (setting_decimal_places > 0) characters += setting_decimal_places + 1; //For example 4: 3 decimal places and the '.'

  if (setting_units == UNITS_LBS) characters += strlen("lbs");
  if (setting_units == UNITS_KG) characters += strlen("kg");

  if (setting_raw_reading_enable == true)
  {
    long rawReading = scale.read_average(setting_average_amount); //Take average reading over a given number of times

    //Establish how much time it takes to do a raw read
    long startTime = millis();
    scale.read_average(setting_average_amount); //Do a dummy read and time it
    averageReadTime = ceil((millis() - startTime));
    sensorReadTime += averageReadTime; //In ms

    characters += strlen("8355808");
  }

  //Serial.print("characterTime: ");
  //Serial.println(ceil((float)characters * characterTime));

  //Combine the total amount of sensor read time with the time it takes to print all the characters
  return (sensorReadTime + ceil((float)characters * characterTime));
}
