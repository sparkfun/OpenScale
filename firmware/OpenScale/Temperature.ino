/*
 Basic functions for the reading of temperature sensors

 There is one on the board and one (optional) off the board
*/


//Read the on board TMP102 digital temperature sensor
//Return celsius
//Code comes from bildr
float getLocalTemperature()
{
  Wire.requestFrom(tmp102Address, 2);

  byte MSB = Wire.read();
  byte LSB = Wire.read();

  //It's a 12bit int, using two's compliment for negative
  int TemperatureSum = ((MSB << 8) | LSB) >> 4;

  float celsius = TemperatureSum * 0.0625;
  return celsius;
}

//Read the remote DS18B20 sensor
//Return celsius
//Code comes from PJRC: http://www.pjrc.com/teensy/td_libs_OneWire.html
float getRemoteTemperature()
{
  //The DS18S20 is read slightly differently than the DS18B20
  //The sealed sensors that SparkFun sells are DS18B20
  //https://www.sparkfun.com/products/11050
  boolean type_s = false;

  //This was moved to the end of the function. We will be calling this function many times so reset the
  //sensor and then tell it to do a temp conversion. This removes the need to delay for a sensor reading.
  remoteSensor.reset();
  remoteSensor.select(remoteSensorAddress); //The address is found at power on
  remoteSensor.write(0x44, 1);        // start conversion, with parasite power on at the end

  //750ms for 12-bit
  //375ms for 11-bit
  //187ms for 10-bit
  //93ms for 9-bit
  delay(100);

  remoteSensor.reset();
  remoteSensor.select(remoteSensorAddress);
  remoteSensor.write(0xBE);         // Read Scratchpad

  byte data[12];
  for (byte i = 0 ; i < 9 ; i++)           // we need 9 bytes
    data[i] = remoteSensor.read();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }

  float celsius = (float)raw / 16.0;
  return (celsius);
}
