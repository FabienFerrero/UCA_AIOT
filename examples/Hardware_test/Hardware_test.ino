/*
 *  _                                   ____                       
 * | |    __ _  ___ _   _ _ __   __ _  / ___| _ __   __ _  ___ ___ 
 * | |   / _` |/ __| | | | '_ \ / _` | \___ \| '_ \ / _` |/ __/ _ \
 * | |__| (_| | (__| |_| | | | | (_| |  ___) | |_) | (_| | (_|  __/
 * |_____\__,_|\___|\__,_|_| |_|\__,_| |____/| .__/ \__,_|\___\___|
 *                                           |_|                   
 * Copyright (C) 2021 Lacuna Space Ltd.
 *
 * Description: Hardware test sketch for LS200 board
 * 
 * License: Revised BSD License, see LICENSE-LACUNA.TXT file included in the project
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Updated by Fabien Ferrero, 13/04/21
 *
 */



#ifndef REGION
#define REGION R_EU868
#endif

//#define DEEPSLEEP

#define WAIT_S 1  // loop delay in seconds

#include <SPI.h>
#include <Wire.h>
#include <RTC.h>
#include <time.h>
//#include <AHT10.h> // https://github.com/enjoyneering/AHT10.git
#include <LTR303.h> // https://github.com/automote/LTR303


// HP203B I2C address is 0x76(118)
#define Addr 0x76

// KX023
union data {
  int16_t data16;
  byte  byteStr[2];//{*DATAL,*DATAH}
};
union data xData;
union data yData;
union data zData;

const int sendorDeviceAddress = 0x1E;//I2C7bitAddressMode was 1F
const int regAddressXOUTL = 0x06;
const int regAddressYOUTL = 0x08;
const int regAddressZOUTL = 0x0A;
const int regAddressODCNTL = 0x1B;
const int regAddressCNTL1 = 0x18;

// KX023 
  byte CNTL1 = 0;

uint8_t readStatus = 0;

//AHT10 myAHT10(AHT10_ADDRESS_0X38);

// Create an LTR303 object, here called "light":
LTR303 light;

// Global variables LTR303:
unsigned char gain;     // Gain setting, values = 0-7 
unsigned char integrationTime;  // Integration ("shutter") time in milliseconds
unsigned char measurementRate;  // Interval between DATA_REGISTERS update

void setup() {
  
  Serial.begin(9600);

  pinMode(LS_LED_BLUE, OUTPUT);
 
  while (!Serial && millis() < 5000);
  
  Serial.println("Initializing");

  Serial.print("Board version: ");
  Serial.println(BOARD_VERSION);

  Serial.print("Configured Region: ");
#if REGION == R_EU868
  Serial.println("Europe 862-870 Mhz");
#elif REGION  == R_US915
  Serial.println("US 902-928 Mhz");
#elif REGION == R_AS923
  Serial.println("Asia 923 Mhz");
#elif REGION == R_IN865
  Serial.println("India 865-867 Mhz");
#else 
  Serial.println("Undefined");  
#endif

  // STM32

  uint32_t uid[3];
  STM32.getUID(uid);
  Serial.print("STM32 UID: ");
  Serial.print(uid[0],HEX);
  Serial.print(" ");
  Serial.print(uid[1],HEX);
  Serial.print(" ");
  Serial.println(uid[2],HEX);

  uint32_t ls_devid = crc32b((uint8_t*)uid);
  Serial.print("STM32 LSDevID: ");
  Serial.println(ls_devid,HEX);

  analogReadResolution(12);

  Wire.begin();

  pinMode(LS_GPS_ENABLE, OUTPUT);
  digitalWrite(LS_GPS_ENABLE, HIGH);
  pinMode(LS_GPS_V_BCKP, OUTPUT);
  digitalWrite(LS_GPS_V_BCKP, HIGH);
  digitalWrite(LS_VERSION_ENABLE, LOW);
  
  pinMode(5, OUTPUT);   // HALL_On_Off
  digitalWrite(5, LOW);

  pinMode(36, OUTPUT);
  digitalWrite(36, LOW);
  
 Serial.println("==========================");

  // KX023
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(0x18);
  Wire.write(0x40);
  Wire.endTransmission();
  //outPutDataLate 50Hz
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(0x1B);
  Wire.write(0x02);
  Wire.endTransmission();
  //sensor WakeUp
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(0x18);
  Wire.write(0xC0);
  Wire.endTransmission();
  
  //setup LPF--------------------------------------------------
  //
  
  //set device standbyMode
  //readCNTL1reg
  
  byte CNTL1 = 0;
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressCNTL1);
  Wire.endTransmission();
  Wire.requestFrom(sendorDeviceAddress, 1);
  CNTL1 = Wire.read();
  //setCNTL1reg
  CNTL1 = CNTL1 & 0b01111111;
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressCNTL1);
  Wire.write(CNTL1);
  Wire.endTransmission();

  //set LPF parameters
  //readODCNTLreg
  byte ODCNTL = 0;
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressODCNTL);
  Wire.endTransmission();
  Wire.requestFrom(sendorDeviceAddress, 1);
  ODCNTL = Wire.read();
  //setODCNTLreg
  ODCNTL = ODCNTL | 0b01000000;//set filter corner frequency set to ODR/2
  ODCNTL = ODCNTL & 0b11110000;//set OutputDataRate 12.5Hz
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressODCNTL);
  Wire.write(ODCNTL);
  Wire.endTransmission();
  
  //set device operating mode
  //readCNTL1reg
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressCNTL1);
  Wire.endTransmission();
  Wire.requestFrom(sendorDeviceAddress, 1);
  CNTL1 = Wire.read();
  //setCNTL1reg
  CNTL1 = CNTL1 | 0b10000000;
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressCNTL1);
  Wire.write(CNTL1);
  Wire.endTransmission();
  //--------------------------------------------------setup LPF

//AHT10
//  while (myAHT10.begin() != true)
//  {
//    Serial.println(F("AHT10 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
//    delay(5000);
//  }
//  Serial.println(F("AHT10 OK"));


// Initialize the LTR303 library
// 100ms   initial startup time required
  delay(100);

  // You can pass nothing to light.begin() for the default I2C address (0x29)
  light.begin();

  unsigned char ID;
  
  if (light.getPartID(ID)) {
    Serial.print("Got Sensor Part ID: 0X");
    Serial.print(ID,HEX);
  }
   else {
    byte error = light.getError();
    printError(error);
  }

  gain = 0;
  light.setControl(gain, false, false);
  unsigned char time = 1;
  light.setMeasurementRate(time,3);
  light.setPowerUp();
}

void loop() {

  digitalWrite(LS_LED_BLUE, HIGH);
  delay(50);
  digitalWrite(LS_LED_BLUE, LOW);
  delay(100);
  digitalWrite(LS_LED_BLUE, HIGH);
  delay(50);
  digitalWrite(LS_LED_BLUE, LOW);

  Serial.println();
  Serial.println("Read sensors");

  uint16_t voltage = (uint16_t)((LS_ADC_AREF / 4.096) * (LS_BATVOLT_R1 + LS_BATVOLT_R2) / LS_BATVOLT_R2 * (float)analogRead(LS_BATVOLT_PIN));
 
  Serial.print("Voltage: ");
  Serial.println(voltage/1000.0f);

  delay(10);

  KX023();
  delay(10);
  HP203B();
//  delay(10);
//  AHT10_();
  delay(10);
  LTR303_(); 


#ifdef DEEPSLEEP
  time_t t;
  struct tm tm;

  t = (time_t)RTC.getEpoch()+20;
  gmtime_r(&t, &tm);
  
  RTC.setAlarmTime(tm.tm_hour, tm.tm_min, tm.tm_sec);
  RTC.setAlarmDay(tm.tm_mday);
  
  RTC.enableAlarm(RTC.MATCH_HHMMSS);
  RTC.attachInterrupt(alarmMatch);

  delay(200);
  LS200_sleep();
  delay(200);
#else 
  delay(WAIT_S*1000);
#endif
  
}

void alarmMatch() { }

void LS200_sleep()  
{
  digitalWrite(LS_GPS_ENABLE, LOW);
  digitalWrite(LS_VERSION_ENABLE, LOW);
  digitalWrite(LS_GPS_V_BCKP, LOW);

  digitalWrite(5, LOW);

  CNTL1 = CNTL1 | 0b00000000;
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressCNTL1);
  Wire.write(CNTL1);
  Wire.endTransmission();
    
  SPI.end();
  delay(10);
  STM32.stop();

  // Sleep...
  
  SPI.begin();
  digitalWrite(LS_GPS_ENABLE, HIGH);
  digitalWrite(LS_VERSION_ENABLE, LOW);
  digitalWrite(LS_GPS_V_BCKP, HIGH);
 // light.setPowerUp();
  
  digitalWrite(5, HIGH);

  CNTL1 = CNTL1 | 0b10000000;
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressCNTL1);
  Wire.write(CNTL1);
  Wire.endTransmission();
}


uint32_t crc32b(uint8_t *data) {
   int i, j;
   uint32_t byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (i < (4*3)) {
      byte = data[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}

void KX023(){
  //readXout
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressXOUTL);
  Wire.endTransmission();
  Wire.requestFrom(sendorDeviceAddress, 2);
  xData.byteStr[0] = Wire.read();
  xData.byteStr[1] = Wire.read();

  //readYout
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressYOUTL);
  Wire.endTransmission();
  Wire.requestFrom(sendorDeviceAddress, 2);
  yData.byteStr[0] = Wire.read();
  yData.byteStr[1] = Wire.read();

  //readZout
  Wire.beginTransmission(sendorDeviceAddress);
  Wire.write(regAddressZOUTL);
  Wire.endTransmission();
  Wire.requestFrom(sendorDeviceAddress, 2);
  zData.byteStr[0] = Wire.read();
  zData.byteStr[1] = Wire.read();

//  //rawDataOutput
//  Serial.print("xdata:");
//  Serial.print(xData.data16, DEC);
//  Serial.print(",");
//  Serial.print("ydata:");
//  Serial.print(yData.data16, DEC);
//  Serial.print(",");
//  Serial.print("zdata:");
//  Serial.println(zData.data16, DEC);
  Serial.print("KX023 Accel: ");
  Serial.print("x:");
  Serial.print(xData.data16, DEC);
  Serial.print(", y:");
  Serial.print(yData.data16, DEC);
  Serial.print(", z:");
  Serial.println(zData.data16, DEC);
}

void HP203B(){
// HP203B 
unsigned int data[6];

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Send OSR and channel setting command
  Wire.write(0x40 |0x04 | 0x00);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(500);

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x10);
  // Stop I2C transmission
  Wire.endTransmission();

  // Request 6 bytes of data
  Wire.requestFrom(Addr, 6);

  // Read 6 bytes of data
  // cTemp msb, cTemp csb, cTemp lsb, pressure msb, pressure csb, pressure lsb
  if (Wire.available() == 6)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
  }

  // Convert the data to 20-bits
  float cTemp = (((data[0] & 0x0F) * 65536) + (data[1] * 256) + data[2]) / 100.00;
  float fTemp = (cTemp * 1.8) + 32;
  float pressure = (((data[3] & 0x0F) * 65536) + (data[4] * 256) + data[5]) / 100.00;

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Send OSR and channel setting command
  Wire.write(0x40 | 0x04 | 0x01);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(500);

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x31);
  // Stop I2C transmission
  Wire.endTransmission();

  // Request 3 bytes of data
  Wire.requestFrom(Addr, 3);

  // Read 3 bytes of data
  // altitude msb, altitude csb, altitude lsb
  if (Wire.available() == 3)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
  }

  // Convert the data to 20-bits
  float altitude = (((data[0] & 0x0F) * 65536) + (data[1] * 256) + data[2]) / 100.00;

  // Output data to serial monitor
  Serial.print("HP203B Altitude : ");
  Serial.print(altitude);
  Serial.println(" m");
  Serial.print("HP203B Pressure : ");
  Serial.print(pressure);
  Serial.println(" Pa");
  Serial.print("HP203B Temperature in Celsius : ");
  Serial.print(cTemp);
  Serial.println(" C");
  //Serial.print("HP203B Temperature in Fahrenheit : ");
  //Serial.print(fTemp);
  //Serial.println(" F");
}

//void AHT10_(){
//  /* DEMO - 1, every temperature or humidity call will read 6 bytes over I2C, total 12 bytes */
//  //Serial.println(F("DEMO 1: read 12-bytes, show 255 if communication error is occurred"));
//  Serial.print(F("AHT10: Temperature: ")); Serial.print(myAHT10.readTemperature()); Serial.println(F(" +-0.3C")); //by default "AHT10_FORCE_READ_DATA"
//  Serial.print(F("AHT10: Humidity...: ")); Serial.print(myAHT10.readHumidity());    Serial.println(F(" +-2%"));   //by default "AHT10_FORCE_READ_DATA"
//}

void LTR303_(){
  int ms = 1000;
  delay(ms);
  unsigned int data0, data1;

  if (light.getData(data0,data1)) {
    // getData() returned true, communication was successful
    
   // Serial.print("data0: ");
   // Serial.println(data0);
   // Serial.print("data1: ");
   // Serial.println(data1);

     double lux;    // Resulting lux value
    boolean good;  // True if neither sensor is saturated
    
    // Perform lux calculation:

    good = light.getLux(gain,integrationTime,data0,data1,lux);
    
    // Print out the results:
    Serial.println("LTR303: ");
    Serial.print(" lux: ");
    Serial.println(lux);
    if (good) Serial.println(" (good)"); else Serial.println(" (BAD)");
  }
  else {
    // getData() returned false because of an I2C error, inform the user.

    byte error = light.getError();
    printError(error);
  }
}

void printError(byte error) {
  // If there's an I2C error, this function will
  // print out an explanation.

  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");
  
  switch(error) {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }

}
