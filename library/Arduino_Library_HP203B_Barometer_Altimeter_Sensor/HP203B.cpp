/**************************************************************************/
/*
        Distributed with a free-will license.
        Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
        HP203B
        This code is designed to work with the HP203B_I2CS I2C Mini Module available from ControlEverything.com.
        https://shop.controleverything.com/products/precision-barometer-and-altimeter?variant=25687549067#tabs-0-product_tabset-2
*/
/**************************************************************************/

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

#include "HP203B.h"

/**************************************************************************/
/*
        Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static uint8_t i2cread(void)
{
    #if ARDUINO >= 100
        return Wire.read();
    #else
        return Wire.receive();
    #endif
}

/**************************************************************************/
/*
        Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static void i2cwrite(uint8_t x)
{
    #if ARDUINO >= 100
        Wire.write((uint8_t)x);
    #else
        Wire.send(x);
    #endif
}

/**************************************************************************/
/*
        Writes 8-bits to the destination register
*/
/**************************************************************************/
static void writeRegister(uint8_t i2cAddress, uint8_t reg)
{
    Wire.beginTransmission(i2cAddress);
    i2cwrite((uint8_t)reg);
    Wire.endTransmission();
}

/**************************************************************************/
/*
    Reads 24-bits from the specified destination register
*/
/**************************************************************************/
static uint32_t readRegister(uint8_t i2cAddress, uint8_t reg)
{
    Wire.beginTransmission(i2cAddress);
    i2cwrite((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom(i2cAddress, (uint8_t)3);
    return (uint32_t)((int32_t)i2cread() << 16) | ((int32_t)i2cread() << 8) | i2cread();
}

/**************************************************************************/
/*
        Instantiates a new HP203B class with appropriate properties
*/
/**************************************************************************/
void HP203B::getAddr_HP203B(uint8_t i2cAddress)
{
    hp_i2cAddress = i2cAddress;
    hp_conversionDelay = HP203B_CONVERSIONDELAY;
}

/**************************************************************************/
/*
        Sets up the Hardware
*/
/**************************************************************************/
bool HP203B::begin()
{
    Wire.begin();
    
    // Reset();
    // delay(hp_conversionDelay);
    
    return true;
}

/**************************************************************************/
/*
        Sets the Soft Reset Command
        The Sevice will Immediately be Reset No Matter What it is Working On
*/
/**************************************************************************/
void HP203B::Reset()
{
    writeRegister(hp_i2cAddress, HP203B_CMD_SOFT_RESET);
    delay(hp_conversionDelay);
}

/**************************************************************************/
/*
        Sets the OSR Command Value to select Decimation Rate of the Internal Digital Filter
*/
/**************************************************************************/
void HP203B::setOSR(hpOSR_t osr)
{
    hp_osr = osr;
}

/**************************************************************************/
/*
        Gets the OSR Command Value to select Decimation Rate of the Internal Digital Filter
 */
/**************************************************************************/
hpOSR_t HP203B::getOSR()
{
    return hp_osr;
}

/**************************************************************************/
/*
        Reads up the Device comprising of a Pressure, Altitude & Temperature Sensor
*/
/**************************************************************************/
void HP203B::Measure_Sensor(void)
{
    // Read all the Sensors
    Measure_Pressure();
    Measure_Altitude();
    Measure_Temperature();
}

/**************************************************************************/
/*
        Reads 20-bits from the destination register
        Reads the results for Digital Pressure Value
*/
/**************************************************************************/
void HP203B::Measure_Pressure()
{
    // Set Up the Configuration for the Pressure Sensor
    uint8_t command =   HP203B_CMD_CONVERT           |  // Convert the Sensor Output to the Digital Values
                        HP203B_CMD_CHNL_PRESTEMP;       // Sensor Pressure and Temperature Channel
    
    command |= hp_osr;       // OSR
    
    // Write the configuration to the Pressure Sensor
    writeRegister(hp_i2cAddress, command);
    
    // Wait for the configuration to complete
    delay(hp_conversionDelay);
    
    // Reads the pressure value
    uint32_t pressure = readRegister(hp_i2cAddress, HP203B_CMD_READ_P);
    hp_sensorData.P = pressure /100.0;
}

/**************************************************************************/
/*
        Reads 20-bits from the destination register
        Reads the results for Digital Altitude Value
*/
/**************************************************************************/
void HP203B::Measure_Altitude()
{
    // Set Up the Configuration for the Altitude Sensor
    uint8_t command =   HP203B_CMD_CONVERT           |  // Convert the Sensor Output to the Digital Values
                        HP203B_CMD_CHNL_PRESTEMP;       // Sensor Pressure and Temperature Channel
    
    command |= hp_osr;       // OSR
    
    // Write the configuration to the Altitude Sensor
    writeRegister(hp_i2cAddress, command);
    
    // Wait for the configuration to complete
    delay(hp_conversionDelay);
    
    // Reads the Altitude value
    uint32_t altitude = readRegister(hp_i2cAddress, HP203B_CMD_READ_A);
    hp_sensorData.A = altitude /100.0;
}

/**************************************************************************/
/*
        Reads 20-bits from the destination register
        Reads the results for Digital Temperature Value
*/
/**************************************************************************/
void HP203B::Measure_Temperature()
{
    // Set Up the Configuration for the Temperature Sensor
    uint8_t command =   HP203B_CMD_CONVERT           |  // Convert the Sensor Output to the Digital Values
                        HP203B_CMD_CHNL_PRESTEMP;       // Sensor Pressure and Temperature Channel
    
    command |= hp_osr;       // OSR
    
    // Write the configuration to the Temperature Sensor
    writeRegister(hp_i2cAddress, command);
    
    // Wait for the configuration to complete
    delay(hp_conversionDelay);
    
    // Reads the Temperature value
    uint32_t temperature = readRegister(hp_i2cAddress, HP203B_CMD_READ_T);
    hp_sensorData.T = temperature /100.0;
}
