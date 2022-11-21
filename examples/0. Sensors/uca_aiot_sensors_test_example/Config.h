#ifndef __AIOT_CONFIG_H__
#define __AIOT_CONFIG_H__

/*********** Library Include ***********/
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

/*********** Hardware ***********/
#define DATA_INTERVAL 1000 //ms

#if defined(GPS_UBLOX) && defined(GPS_QUECTEL)
#error Inappropriate GPS type defined. Check the config.h file!
#endif

/*********** Code configuration ***********/
#define I2C_LTR303_ADDRESS  0X29
#define I2C_KX023_ADDRESS   0X1E
#define I2C_HP203B_ADDRESS  0X77

#define I2C_GPS_QUECTEL_ADDRESS     0X10
#define I2C_GPS_UBLOX_ADDRESS       0X42
#ifdef GPS_UBLOX
#define I2C_GPS_ADDRESS I2C_GPS_UBLOX_ADDRESS
#endif
#ifdef GPS_QUECTEL
#define I2C_GPS_ADDRESS I2C_GPS_QUECTEL_ADDRESS
#endif

/*********** Data Type Define ***********/


/*********** Others ***********/

#endif /* __AIOT_CONFIG_H__ */
