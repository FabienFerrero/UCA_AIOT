/*********** Uncomment the GPS type you have on your board! ***********/
#define GPS_UBLOX
// #define GPS_QUECTEL

#include "Config.h"
#include <RTC.h>
#include <time.h>
#include <EEPROM.h>
#include <HP203B.h>                         // https://github.com/ncdcommunity/Arduino_Library_HP203B_Barometer_Altimeter_Sensor
#include <LTR303.h>                         // https://github.com/automote/LTR303
#include <Adafruit_SGP30.h>                 // https://github.com/adafruit/Adafruit_SGP30
#include <SparkFun_Ublox_Arduino_Library.h> // http://librarymanager/All#SparkFun_Ublox
#include <ICM_20948.h>                      // https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary
#include <MicroNMEA.h>                      // http://librarymanager/All#MicroNMEA

/*********** Global variables ***********/
unsigned long currentMillis = 0, getSensorDataPrevMillis = 0, getGPSPrevMillis = 0;
File myFile;

// LTR303
LTR303 light;
unsigned char gain;            // Gain setting, values = 0-7
unsigned char integrationTime; // Integration ("shutter") time in milliseconds
unsigned char measurementRate; // Interval between DATA_REGISTERS update

// HP203
HP203B hp;
double referencePressure;

// SGP30
Adafruit_SGP30 sgp;
int8_t SGP_counter = 0;

// ICM
ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object

// GPS
SFE_UBLOX_GPS myGPS;

uint32_t quectelDelayTime = 500;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

void setup(void)
{
  // GPIO
  pinMode(LS_LED_BLUE, OUTPUT);
  pinMode(LS_GPS_ENABLE, OUTPUT);
  pinMode(LS_GPS_V_BCKP, OUTPUT);
  pinMode(SD_ON_OFF, OUTPUT);
  pinMode(LS_VERSION_ENABLE, OUTPUT);

  // Check GPS Firmware Upgrade Mode
  pinMode(LS_USER_BUTTON, INPUT);
  check_GPSFirmwareUpload();

  // Normal running
  digitalWrite(LS_GPS_ENABLE, HIGH);
  digitalWrite(LS_GPS_V_BCKP, HIGH);
  digitalWrite(SD_ON_OFF, HIGH);
  digitalWrite(LS_VERSION_ENABLE, HIGH);

  delay(100);

  randomSeed(analogRead(LS_VERSION_MEAS));
  delay(20);
  digitalWrite(LS_VERSION_ENABLE, LOW);

  // Serial
  Serial.begin(115200);
  while (!Serial && (millis() < 5000))
  {
  }
  Serial.println("======= DK_AIoT Hardware Test! =======");
  Serial.println("Initializing");

  // STM32
  uint32_t uid[3];
  STM32.getUID(uid);
  Serial.print("STM32 UID: ");
  Serial.print(uid[0], HEX);
  Serial.print(" ");
  Serial.print(uid[1], HEX);
  Serial.print(" ");
  Serial.println(uid[2], HEX);

  uint32_t ls_devid = crc32b((uint8_t *)uid);
  Serial.print("STM32 LSDevID: ");
  Serial.println(ls_devid, HEX);

  analogReadResolution(12);
  Wire.begin();

  delay(200);

  // LTR303
  Serial.println("==============================================");
  light.begin();
  unsigned char LTR303_ID;
  if (light.getPartID(LTR303_ID))
  {
    Serial.print("LTR303: Got Sensor Part ID: 0X");
    Serial.println(LTR303_ID, HEX);
  }
  else
  {
    byte error = light.getError();
    LTR_printError(error);
  }
  gain = 0;
  Serial.println("LTR303: Setting Gain...");
  light.setControl(gain, false, false);
  unsigned char time = 1;
  Serial.println("LTR303: Set timing...");
  light.setMeasurementRate(time, 3);
  Serial.println("LTR303: Powerup...");
  light.setPowerUp();

  // HP203
  Serial.println("==============================================");
  Serial.println("HP203 Initializing...");
  hp.getAddr_HP203B(I2C_HP203B_ADDRESS);
  hp.setOSR(OSR_4096); // OSR=4096
  hp.begin();

  delay(300);
  Wire.beginTransmission(I2C_HP203B_ADDRESS);
  byte error = Wire.endTransmission();
  if (error != 0)
  {
    Serial.println("HP203B Disconnected!");
  }

  // SGP
  Serial.println("==============================================");
  if (!sgp.begin())
  {
    Serial.println("SGP30: Sensor not found");
  }
  Serial.print("SGP30: Found sensor with SN #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // ICM
  Serial.println("==============================================");
  bool ICM_initialized = false;
  for (int i = 1; i <= 5; i++)
  {
    myICM.begin(Wire, 0);
    Serial.print("ICM: ");
    Serial.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      Serial.println("ICM: Try again....");
      Serial.print(i);
    }
    else
    {
      Serial.println("ICM: Successful");
      ICM_initialized = true;
    }
  }
  if (!ICM_initialized)
  {
    Serial.print("ICM: Failed");
  }

  // GPS
  Serial.println("==============================================");
#if defined(GPS_UBLOX)
  if (myGPS.begin() == false) // Connect to the Ublox module using Wire port
  {
    Serial.println(F("Ublox GPS not detected at default I2C address. Please check wiring."));
  }
  else
  {
    Serial.println(F("Ublox GPS OK"));
  }
  myGPS.setI2COutput(COM_TYPE_UBX); // Set the I2C port to output UBX only (turn off NMEA noise)
  myGPS.saveConfiguration();        // Save the current settings to flash and BBR

#endif

#if defined(GPS_QUECTEL)
  Wire.beginTransmission(I2C_GPS_ADDRESS);
  error = Wire.endTransmission();
  if (error == 0)
  {
    Serial.println(F("Quectel GPS: OK"));
  }
  else
  {
    Serial.println(F("Quectel GPS: FAILED"));
  }
#endif

  // SDCard
  Serial.println("\n\n\n\n============================ SDCard_test ============================\n\n\n\n");
  SDCard_test();

  // Sensors
  Serial.println("\n\n\n\n============================ Sensor_Test ============================\n\n\n\n");
  Sensor_Test();

  Serial.println("\n\n\n\n==============================================\n\n\n\n");
}

void loop()
{
  delay(50);
}

void LTR_printError(byte error)
{
  // If there's an I2C error, this function will
  // print out an explanation.

  Serial.print("LTR303: I2C error: ");
  Serial.print(error, DEC);
  Serial.print(", ");

  switch (error)
  {
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

uint32_t crc32b(uint8_t *data)
{
  int i, j;
  uint32_t byte, crc, mask;

  i = 0;
  crc = 0xFFFFFFFF;
  while (i < (4 * 3))
  {
    byte = data[i]; // Get next byte.
    crc = crc ^ byte;
    for (j = 7; j >= 0; j--)
    { // Do eight times.
      mask = -(crc & 1);
      crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }
    i = i + 1;
  }
  return ~crc;
}

void GPS_showData(void)
{
#if defined(GPS_UBLOX)
  Serial.print("GPS time: ");
  Serial.print(myGPS.getYear());
  Serial.print("-");
  Serial.print(myGPS.getMonth());
  Serial.print("-");
  Serial.print(myGPS.getDay());
  Serial.print(" ");
  Serial.print(myGPS.getHour());
  Serial.print(":");
  Serial.print(myGPS.getMinute());
  Serial.print(":");
  Serial.println(myGPS.getSecond());

  Serial.print("  GPS position: ");
  Serial.print(myGPS.getLatitude() / 1e7, 4);
  Serial.print(", ");
  Serial.print(myGPS.getLongitude() / 1e7, 4);
  Serial.print(" (");
  Serial.print(myGPS.getSIV());
  Serial.println(" satellites)");
#endif

#if defined(GPS_QUECTEL)
  Serial.print("Valid fix: ");
  Serial.println(nmea.isValid() ? "yes" : "no");

  Serial.print("Nav. system: ");
  if (nmea.getNavSystem())
    Serial.println(nmea.getNavSystem());
  else
    Serial.println("none");

  Serial.print("HDOP: ");
  Serial.println(nmea.getHDOP() / 10., 1);

  Serial.print("  GPS time: ");
  Serial.print(nmea.getYear());
  Serial.print("-");
  Serial.print(nmea.getMonth());
  Serial.print("-");
  Serial.print(nmea.getDay());
  Serial.print(" ");
  Serial.print(nmea.getHour());
  Serial.print(":");
  Serial.print(nmea.getMinute());
  Serial.print(":");
  Serial.println(nmea.getSecond());

  uint32_t unixt = unixTimestamp(nmea.getYear(), nmea.getMonth(), nmea.getDay(), nmea.getHour(), nmea.getMinute(), nmea.getSecond());
  Serial.print("  Unix time GPS: ");
  Serial.println(unixt);

  long latitude = nmea.getLatitude();
  long longitude = nmea.getLongitude();
  long altitude = 0;
  nmea.getAltitude(altitude);

  Serial.print("  GPS position: ");
  Serial.print(latitude / 1.0e6, 4);
  Serial.print(", ");
  Serial.print(longitude / 1.0e6, 4);
  Serial.print(" alt: ");
  Serial.print(altitude / 1.0e3, 2);
  Serial.print(" (");
  Serial.print(nmea.getNumSatellites());
  Serial.println(" satellites)");
#endif
}

uint32_t SGP_getAbsoluteHumidity(float temperature, float humidity)
{
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
  return absoluteHumidityScaled;
}

void ICM_printScaledAGMT(ICM_20948_I2C *sensor)
{
  Serial.print("ICM: Scaled. Acc (mg) [ ");
  printFormattedFloat(sensor->accX(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->accY(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->accZ(), 5, 2);
  Serial.print(" ], Gyr (DPS) [ ");
  printFormattedFloat(sensor->gyrX(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->gyrY(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->gyrZ(), 5, 2);
  Serial.print(" ], Mag (uT) [ ");
  printFormattedFloat(sensor->magX(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->magY(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->magZ(), 5, 2);
  Serial.print(" ], Tmp (C) [ ");
  printFormattedFloat(sensor->temp(), 5, 2);
  Serial.print(" ]");
  Serial.println();
}

void printFormattedFloat(float val, uint8_t leading, uint8_t decimals)
{
  float aval = abs(val);
  if (val < 0)
  {
    Serial.print("-");
  }
  else
  {
    Serial.print(" ");
  }
  for (uint8_t indi = 0; indi < leading; indi++)
  {
    uint32_t tenpow = 0;
    if (indi < (leading - 1))
    {
      tenpow = 1;
    }
    for (uint8_t c = 0; c < (leading - 1 - indi); c++)
    {
      tenpow *= 10;
    }
    if (aval < tenpow)
    {
      Serial.print("0");
    }
    else
    {
      break;
    }
  }
  if (val < 0)
  {
    Serial.print(-val, decimals);
  }
  else
  {
    Serial.print(val, decimals);
  }
}

void check_GPSFirmwareUpload(void)
{
  int lastBtnState, btnState;
  delay(50);

  for (int i = 0; i < 5; i++)
  {
    lastBtnState = digitalRead(LS_USER_BUTTON);
    delay(50);
    btnState = digitalRead(LS_USER_BUTTON);

    if (lastBtnState == btnState)
    {
      if (lastBtnState == LOW)
      {
        GPSFirmwareUploadMode();
      }
      else if (lastBtnState == HIGH)
      {
        break;
      }
    }
  }
}

void GPSFirmwareUploadMode(void)
{
  Serial.println("\n\n\n\n\n");
  Serial.println("> GPSFirmwareUploadMode: Triggered!");

  digitalWrite(LS_GPS_ENABLE, HIGH);
  digitalWrite(LS_GPS_V_BCKP, HIGH);
  digitalWrite(SD_ON_OFF, LOW);
  digitalWrite(LS_VERSION_ENABLE, LOW);

  digitalWrite(LS_LED_BLUE, HIGH);

  while (1)
  {
  }
}

void quectel_getData(void)
{
  char revChar = 0;

  Wire.requestFrom(I2C_GPS_QUECTEL_ADDRESS, 255);
  while (Wire.available())
  {
    revChar = Wire.read();
    nmea.process(revChar);

    if (revChar == 0x0A) // End character | Garbage bytes
    {
      quectelDelayTime = 500; // 500ms
      return;
    }
  }
  quectelDelayTime = 5; // 5ms (Minimum delay time for slave to prepare the next NMEA packet is 2ms)
}

unsigned long unixTimestamp(int year, int month, int day, int hour, int min, int sec)
{
  const short days_since_beginning_of_year[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int leap_years = ((year - 1) - 1968) / 4 - ((year - 1) - 1900) / 100 + ((year - 1) - 1600) / 400;
  long days_since_1970 = (year - 1970) * 365 + leap_years + days_since_beginning_of_year[month - 1] + day - 1;
  if ((month > 2) && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
    days_since_1970 += 1; /* +leap day, if year is a leap year */
  return sec + 60 * (min + 60 * (hour + 24 * days_since_1970));
}

void SDCard_test(void)
{
  uint8_t sdInitDoneFlag = 0;
  char randomCharArray[4];

  randomCharArray[0] = random(65, 90);
  randomCharArray[1] = random(97, 122);
  randomCharArray[2] = random(48, 57);
  randomCharArray[3] = 0;

  Serial.print("SDCard: Initializing...");
  if (!SD.begin(10))
  {
    Serial.println("SDCard: Initialization failed!");
    sdInitDoneFlag = 1;
  }

  myFile = SD.open("test.txt", FILE_WRITE & (~O_APPEND));
  if (myFile)
  {
    Serial.println("SDCard: Writing to test.txt...");
    Serial.print("Text-to-write: ");
    Serial.println((char *)randomCharArray);

    myFile.print((char *)randomCharArray);
    myFile.close();
  }
  else
  {
    Serial.println("SDCard: Error opening test.txt");
  }

  myFile = SD.open("test.txt");
  if (myFile)
  {
    Serial.println("SDCard: Reopen test.txt for reading");

    int i = 0, flag = 0;
    for (i = 0; (i < 4) && (myFile.available()); i++)
    {
      char c_temp = myFile.read();
      Serial.print(c_temp);
      if (c_temp != randomCharArray[i])
      {
        flag = 1;
        break;
      }
    }

    if (flag == 0)
    {
      Serial.println("\nSDCard: Read/Write OK!");
    }
    else
    {
      Serial.println("\nSDCard: Reading failed! Wrong value!");
    }
  }
  else
  {
    Serial.println("SDCard: Reading failed!");
  }

  delay(500);
  myFile.close();
}

void Sensor_Test(void)
{
  uint8_t index = 0;

  while (index < 5)
  {
    currentMillis = millis();

    // Parse data from Quectel
#if defined(GPS_QUECTEL)
    if (currentMillis - getGPSPrevMillis > quectelDelayTime)
    {
      getGPSPrevMillis = currentMillis;

      quectel_getData();
    }
#endif

    // Print sensor & gps data
    if (currentMillis - getSensorDataPrevMillis > DATA_INTERVAL)
    {
      index++;
      getSensorDataPrevMillis = currentMillis;

      // LTR303
      unsigned int data0, data1;

      Serial.println("**********************************************");
      if (light.getData(data0, data1))
      {
        Serial.print("LTR303: data0: ");
        Serial.println(data0);
        Serial.print("data1: ");
        Serial.println(data1);

        double lux;   // Resulting lux value
        boolean good; // True if neither sensor is saturated

        good = light.getLux(gain, integrationTime, data0, data1, lux);

        Serial.print("LTR303: lux: ");
        Serial.println(lux);
        if (good)
          Serial.println(" (good)");
        else
          Serial.println(" (BAD)");
      }
      else
      {
        byte error = light.getError();
        LTR_printError(error);
      }

      // HP203
      hp.Measure_Sensor();

      Serial.print("HP203: Pressure ");
      Serial.print(hp.hp_sensorData.P);
      Serial.println(" hPa");
      Serial.print("HP203: Altitude ");
      Serial.print(hp.hp_sensorData.A);
      Serial.println(" m");
      Serial.print("HP203: Temperature ");
      Serial.print(hp.hp_sensorData.T);
      Serial.println(" °C");

      // SGP
      Serial.println("**********************************************");
      if (!sgp.IAQmeasure())
      {
        Serial.println("SGP: Measurement failed");
      }
      Serial.print("SGP: TVOC ");
      Serial.print(sgp.TVOC);
      Serial.print(" ppb\t");
      Serial.print("eCO2 ");
      Serial.print(sgp.eCO2);
      Serial.println(" ppm");

      if (!sgp.IAQmeasureRaw())
      {
        Serial.println("SGP: Raw Measurement failed");
      }
      Serial.print("SGP: Raw H2 ");
      Serial.print(sgp.rawH2);
      Serial.print(" \t");
      Serial.print("Raw Ethanol ");
      Serial.println(sgp.rawEthanol);

      SGP_counter++;
      if (SGP_counter >= 30)
      {
        SGP_counter = 0;

        uint16_t TVOC_base, eCO2_base;
        if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
        {
          Serial.println("SGP: Failed to get baseline readings");
        }
        Serial.print("SGP Baseline values: eCO2 0x");
        Serial.print(eCO2_base, HEX);
        Serial.print(" & TVOC 0x");
        Serial.println(TVOC_base, HEX);
      }

      // ICM
      Serial.println("**********************************************");
      if (myICM.dataReady())
      {
        myICM.getAGMT(); // The values are only updated when you call 'getAGMT'
        //    printRawAGMT( myICM.agmt );     // Uncomment this to see the raw values, taken directly from the agmt structure
        ICM_printScaledAGMT(&myICM); // This function takes into account the scale settings from when the measurement was made to calculate the values with units
      }
      else
      {
        Serial.println("ICM: Waiting for data");
      }

      // GPS
      Serial.println("**********************************************");
      GPS_showData();
    }
  }
}
