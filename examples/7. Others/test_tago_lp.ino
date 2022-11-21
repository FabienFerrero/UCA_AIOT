#include "Config.h"
#include <LibLacuna.h>
#include <SPI.h>
#include <time.h>
#include <RTC.h>
#include <EEPROM.h>
#include <HP203B.h>                         // https://github.com/ncdcommunity/Arduino_Library_HP203B_Barometer_Altimeter_Sensor
//#include <LTR303.h>
#include <Adafruit_SGP30.h>

#ifndef REGION
#define REGION R_EU868
#endif


// Keys and device address are MSB
static byte networkKey[] = {0x7A, 0xB2, 0xF4, 0xBC, 0x9B, 0x04, 0xE7, 0x09, 0x10, 0x85, 0x49, 0x6A, 0xD8, 0xAC, 0x17, 0x57};
static byte appKey[] = {0x72, 0xCA, 0xC4, 0xA9, 0xF9, 0x8E, 0x12, 0x63, 0x49, 0x29, 0x56, 0x21, 0xA9, 0xB5, 0x7A, 0x0D};
static byte deviceAddress[] = {0x26, 0x0B, 0x58, 0x66};

static char payload[255];
static int payloadLength;

static lsLoraWANParams loraWANParams;
static lsLoraSatTxParams SattxParams;
static lsLoraTxParams txParams;

// HP203
HP203B hp;
double referencePressure;
struct HP203_data {
  int16_t temperature;
  uint16_t pression;
  uint16_t altitude;
};

// SGP30
Adafruit_SGP30 sgp;
int8_t SGP_counter = 0;
struct SGP30_data {
  uint16_t TVOC;
  uint16_t eCO2;
  uint16_t rawH2;
  uint16_t rawEthanol;
};

// Low Power
uint32_t last_epoch;
uint32_t alarmepoch;
uint32_t txepoch = 0;
uint16_t sleeptimesec = 15; // Sleep 20 seconds
int32_t sleepseconds;

void setup() {
  // GPIO
  pinMode(LS_LED_BLUE, OUTPUT);
  pinMode(LS_GPS_ENABLE, OUTPUT);
  pinMode(LS_GPS_V_BCKP, OUTPUT);
  pinMode(SD_ON_OFF, OUTPUT);
  pinMode(LS_VERSION_ENABLE, OUTPUT);

  // Normal running
  digitalWrite(LS_GPS_ENABLE, HIGH);
  digitalWrite(LS_GPS_V_BCKP, HIGH);
  digitalWrite(SD_ON_OFF, HIGH);
  digitalWrite(LS_VERSION_ENABLE, HIGH);

  delay(100);

  randomSeed(analogRead(LS_VERSION_MEAS));
  delay(20);
  digitalWrite(LS_VERSION_ENABLE, LOW);

  analogReadResolution(12);
  Wire.begin();

  delay(200);

  // LoRa
  // SX1262 configuration for lacuna LS200 board
  lsSX126xConfig cfg;
  lsCreateDefaultSX126xConfig(&cfg, BOARD_VERSION);

  // Special configuration for DKAIoT Board
  cfg.nssPin = E22_NSS;                           //19
  cfg.resetPin = E22_NRST;                        //14
  cfg.antennaSwitchPin = E22_RXEN;                //1
  cfg.busyPin = E22_BUSY;                         //2
  cfg.dio1Pin = E22_DIO1;                         //39

  // Initialize SX1262
  int result = lsInitSX126x(&cfg, REGION);

  // LoRaWAN session parameters
  lsCreateDefaultLoraWANParams(&loraWANParams, networkKey, appKey, deviceAddress);
  loraWANParams.txPort = 1;
  loraWANParams.rxEnable = true;
 
  // transmission parameters for Lacuna satellites
  lsCreateDefaultLoraSatTxParams(&SattxParams, REGION);

  // transmission parameters for terrestrial LoRa
  lsCreateDefaultLoraTxParams(&txParams, REGION);
  txParams.spreadingFactor = lsLoraSpreadingFactor_7;
  txParams.frequency = 868100000;
  //txParams.power = 1;

  //=================== Sensors ===========================
  // HP203
  hp.getAddr_HP203B(I2C_HP203B_ADDRESS);
  hp.setOSR(OSR_4096); // OSR=4096
  hp.begin();

  delay(300);
  Wire.beginTransmission(I2C_HP203B_ADDRESS);
  byte error = Wire.endTransmission();
  if (error != 0)
  {
    //Serial.println("HP203B Disconnected!");
  }

  // SGP30
  sgp.begin();
}

void loop() {
  digitalWrite(LS_LED_BLUE, HIGH);
  delay(100);
  digitalWrite(LS_LED_BLUE, LOW);
  delay(100);
  digitalWrite(LS_LED_BLUE, HIGH);
  delay(100);
  digitalWrite(LS_LED_BLUE, LOW);

  last_epoch = RTC.getEpoch();
  
  HP203_data data_HP = HP203_meas();
  SGP30_data data_SGP = SGP30_meas();

  unsigned char mydata[12];
  mydata[0] = 0x1; // CH1
  mydata[1] = 0x67; // Temperature
  mydata[2] = (10*data_HP.temperature) >> 8;
  mydata[3] = (10*data_HP.temperature);
  mydata[4] = 0x2; // CH2
  mydata[5] = 0x73; // Barometric
  mydata[6] = (10*data_HP.pression) >> 8;
  mydata[7] = (10*data_HP.pression);
  mydata[8] = 0x3; // CH3
  mydata[9] = 0x02; // AnalogInput
  mydata[10] = (100*data_HP.altitude) >> 8;
  mydata[11] = (100*data_HP.altitude);

  mydata[12] = 0x4; // CH4
  mydata[13] = 0x02; // AnalogInput
  mydata[14] = (100*data_SGP.TVOC) >> 8;
  mydata[15] = (100*data_SGP.TVOC);
  mydata[16] = 0x5; // CH5
  mydata[17] = 0x02; // AnalogInput
  mydata[18] = (10*data_SGP.eCO2) >> 8;
  mydata[19] = (10*data_SGP.eCO2);

  
  // Sending LoRa message 
  int lora_result  = lsSendLoraWAN(&loraWANParams, &txParams, mydata, sizeof(mydata));
  
  //last_epoch = RTC.getEpoch();
  alarmepoch = last_epoch + sleeptimesec;
  sleepseconds = alarmepoch - RTC.getEpoch();
  if (alarmepoch > RTC.getEpoch())
  {
    time_t t;
    struct tm tm;

    t = (time_t)alarmepoch;
    gmtime_r(&t, &tm);
    RTC.setAlarmTime(tm.tm_hour, tm.tm_min, tm.tm_sec);
    RTC.setAlarmDay(tm.tm_mday);
    
    RTC.enableAlarm(RTC.MATCH_HHMMSS);
    RTC.attachInterrupt(alarmMatch);

    // put LS200 board in deepsleep
    lp_sleep();
    delay(100);
  }
}
// Low-Power
void alarmMatch() { }

void lp_sleep()
{
  digitalWrite(LS_GPS_ENABLE, LOW);
  digitalWrite(LS_VERSION_ENABLE, LOW);
  digitalWrite(LS_GPS_V_BCKP, HIGH);
  SPI.end();
  sgp.softReset();
  delay(10);
  STM32.stop();

  SPI.begin();
  digitalWrite(LS_GPS_ENABLE, HIGH);//HIGH
  digitalWrite(LS_VERSION_ENABLE, LOW);
  digitalWrite(LS_GPS_V_BCKP, HIGH);//HIGH
}

// Ma fonction pour faire une mesure HP203B
HP203_data HP203_meas()
{
  HP203_data data;
  
  hp.Measure_Sensor();
  data.temperature = hp.hp_sensorData.T;
  data.pression = hp.hp_sensorData.P;
  data.altitude = hp.hp_sensorData.A;

  return data;
}

SGP30_data SGP30_meas()
{
  SGP30_data data;
  
  sgp.IAQmeasure();
  data.TVOC = sgp.TVOC;
  data.eCO2 = sgp.eCO2;
  data.rawH2 = sgp.rawH2;
  data.rawEthanol = sgp.rawEthanol;

  SGP_counter++;
  if (SGP_counter >= 30)
  {
    SGP_counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
    {
      //Serial.println("SGP: Failed to get baseline readings");
    }
  }
  
  return data;
}
