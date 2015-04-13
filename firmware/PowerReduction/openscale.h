//Arduino pin connections to the DAT and CLK pins on the HX711 IC
#define DAT  2
#define CLK  3
#define AMP_EN A2 //The enable pin to the 5V regulator for the HX711 IC

#define tmp102Address 0x49 //This is the 7-bit address of the TMP102. On OpenScale it has ADR0 tied to VCC.

//Internal EEPROM locations for user settings
#define LOCATION_MASS_UNITS             0x01
#define LOCATION_REPORT_RATE_MSB                (LOCATION_MASS_UNITS + 1)
#define LOCATION_REPORT_RATE_LSB                (LOCATION_MASS_UNITS + 2)
#define LOCATION_CALIBRATION_FACTOR_MSB         (LOCATION_MASS_UNITS + 3)
#define LOCATION_CALIBRATION_FACTOR_MIDHIGH     (LOCATION_MASS_UNITS + 4)
#define LOCATION_CALIBRATION_FACTOR_MIDLOW      (LOCATION_MASS_UNITS + 5)
#define LOCATION_CALIBRATION_FACTOR_LSB         (LOCATION_MASS_UNITS + 6)
#define LOCATION_BAUD_MSB	                (LOCATION_MASS_UNITS + 7)
#define LOCATION_BAUD_MIDHIGH	                (LOCATION_MASS_UNITS + 8)
#define LOCATION_BAUD_MIDLOW	                (LOCATION_MASS_UNITS + 9)
#define LOCATION_BAUD_LSB        	        (LOCATION_MASS_UNITS + 10)
#define LOCATION_TARE_POINT_MSB                 (LOCATION_MASS_UNITS + 11)
#define LOCATION_TARE_POINT_MIDHIGH             (LOCATION_MASS_UNITS + 12)
#define LOCATION_TARE_POINT_MIDLOW              (LOCATION_MASS_UNITS + 13)
#define LOCATION_TARE_POINT_LSB                 (LOCATION_MASS_UNITS + 14)
#define LOCATION_TIMESTAMP                      (LOCATION_MASS_UNITS + 15)
#define LOCATION_DECIMAL_PLACES                 (LOCATION_MASS_UNITS + 16)
#define LOCATION_AVERAGE_AMOUNT                 (LOCATION_MASS_UNITS + 17)
#define LOCATION_LOCAL_TEMP_ENABLE              (LOCATION_MASS_UNITS + 18)
#define LOCATION_REMOTE_TEMP_ENABLE             (LOCATION_MASS_UNITS + 19)
#define LOCATION_STATUS_ENABLE                  (LOCATION_MASS_UNITS + 20)

//Arduino doesn't properly handle bauds lower than 500bps
#define BAUD_MIN  2400
#define BAUD_MAX  1000000

#define UNITS_KG  0
#define UNITS_LBS 1

