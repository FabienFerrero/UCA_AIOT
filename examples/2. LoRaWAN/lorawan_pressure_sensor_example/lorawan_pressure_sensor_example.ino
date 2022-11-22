/*
       __          ____        _____                       __    _ __
      / /   ____  / __ \____ _/ ___/____  ____ _________  / /   (_) /_
     / /   / __ \/ /_/ / __ `/\__ \/ __ \/ __ `/ ___/ _ \/ /   / / __ \
    / /___/ /_/ / _, _/ /_/ /___/ / /_/ / /_/ / /__/  __/ /___/ / /_/ /
   /_____/\____/_/ |_|\__,_//____/ .___/\__,_/\___/\___/_____/_/_.___/
                                /_/
   Author: m1nhle, mtnguyen
   Lib jointy developed by UCA & RFThings
*/

/* Support REGION
    RFT_REGION_EU863_870
    RFT_REGION_US902_928
    RFT_REGION_CN470_510
    RFT_REGION_AU915_928
    RFT_REGION_AS920_923
    RFT_REGION_AS923_925
    RFT_REGION_KR920_923
    RFT_REGION_IN865_867
*/

/****** LIBRARIES INCLUDE ******/
#include <RFThings.h>
#include <rfthings_sx126x.h>
#include <Wire.h>
#include <HP203B.h>

/****** LORA / LORWAN PARAMETER ******/
#define TX_INTERVAL (1000*60) // every minute

uint32_t current_timestamp = 0;
uint32_t sending_timestamp = 0;

// Keys and device address are MSB
static uint8_t nwkS_key[] = {0x33, 0xD5, 0xE5, 0x96, 0xB4, 0x45, 0xCC, 0x64, 0x4A, 0x0D, 0xA8, 0xA0, 0xA1, 0xDD, 0x06, 0x47};
static uint8_t appS_key[] = {0x77, 0xA1, 0xB4, 0x7C, 0x5E, 0x4A, 0xBF, 0x10, 0xFC, 0x3D, 0xCE, 0x1A, 0x29, 0x1B, 0x3E, 0xCE};
static uint8_t dev_addr[] = {0x26, 0x0B, 0xA0, 0x65};

rfthings_sx126x sx126x(E22_NSS, E22_NRST, E22_BUSY, E22_DIO1, E22_RXEN);
rft_status_t status;

char payload[255];
uint32_t payload_len;

/****** PRESSURE SENSOR ******/
#define READING_INTERVAL 5000
uint32_t reading_timestamp = 0;

HP203B hp;
double referencePressure;


/****** MAIN PROGRAM ******/
void setup(void)
{
  // GPIO Initialization
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LS_GPS_ENABLE, OUTPUT);
  pinMode(LS_GPS_V_BCKP, OUTPUT);
  pinMode(SD_ON_OFF, OUTPUT);
  pinMode(LS_VERSION_ENABLE, OUTPUT);

  digitalWrite(LS_GPS_ENABLE, HIGH);
  digitalWrite(LS_GPS_V_BCKP, HIGH);
  digitalWrite(SD_ON_OFF, HIGH);
  digitalWrite(LS_VERSION_ENABLE, HIGH);

  Serial.begin(9600);
  while (!Serial && (millis() < 5000)) {}

  // Init SX126x
  Serial.println("#### SX126X Initialize ####");
  status = sx126x.init(RFT_REGION_EU863_870);
  Serial.print("SX126x initialization: ");
  Serial.println(rft_status_to_str(status));

  // LoRaWAN parameters
  sx126x.set_lorawan_activation_type(RFT_LORAWAN_ACTIVATION_TYPE_ABP);
  sx126x.set_application_session_key(appS_key);
  sx126x.set_network_session_key(nwkS_key);
  sx126x.set_device_address(dev_addr);

  // LoRa parameters
  sx126x.set_tx_power(14);
  sx126x.set_frequency(868100000);
  sx126x.set_spreading_factor(RFT_LORA_SPREADING_FACTOR_7);
  sx126x.set_bandwidth(RFT_LORA_BANDWIDTH_125KHZ);
  sx126x.set_coding_rate(RFT_LORA_CODING_RATE_4_5);
  sx126x.set_syncword(RFT_LORA_SYNCWORD_PUBLIC);

  delay(1000); // Wait for the HP203B sensor to power ON
  hp.getAddr_HP203B(HP203B_ADDRESS_UPDATED); // 0x77
  hp.setOSR(OSR_4096);            // OSR=4096
  hp.begin();

  delay(500);

  current_timestamp = millis();
  reading_timestamp = current_timestamp;
  sending_timestamp = current_timestamp;
}


void loop(void)
{
  current_timestamp = millis();

  if (current_timestamp - reading_timestamp > READING_INTERVAL)
  {
    reading_timestamp = current_timestamp;

    byte error;
    int8_t address;

    address = hp.hp_i2cAddress;
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {

      Serial.println("Getting Pressure, Altitude and Temperature Readings from HP203B");
      Serial.println(" ");
      delay(300);

      // Read the Pressure, Altitude and Temperature
      hp.Measure_Sensor();

      // Output Data to Screen
      Serial.print("Digital Pressure Reading: ");
      Serial.print(hp.hp_sensorData.P);
      Serial.println(" hPa");
      Serial.print("Digital Altitude Reading: ");
      Serial.print(hp.hp_sensorData.A);
      Serial.println(" m");
      Serial.print("Temperature Reading in Celsius: ");
      Serial.print(hp.hp_sensorData.T);
      Serial.println(" °C");
      Serial.print("Temperature Reading in Fahrenheit: ");
      Serial.print(hp.hp_sensorData.T * 1.8 + 32);
      Serial.println(" °F");
      Serial.println(" ");
      Serial.println("        ***************************        ");
      Serial.println(" ");
    }
    else
    {
      Serial.println("HP203B Disconnected!");
      Serial.println(" ");
      Serial.println("        ************        ");
      Serial.println(" ");
    }
  }

  if (current_timestamp - sending_timestamp > TX_INTERVAL)
  {
    sending_timestamp = current_timestamp;
    build_payload();
    send_lorawan();
  }
}

/****** OTHER FUNCTIONs ******/
void build_payload(void)
{
  int32_t value_p = hp.hp_sensorData.P * 1000.0;
  int32_t value_a = hp.hp_sensorData.A * 1000.0;
  int32_t value_t = hp.hp_sensorData.T * 1000.0;

  payload[0] = (value_p >> 0) & 0xff;
  payload[1] = (value_p >> 8) & 0xff;
  payload[2] = (value_p >> 16) & 0xff;
  payload[3] = (value_p >> 24) & 0xff;

  payload[4] = (value_a >> 0) & 0xff;
  payload[5] = (value_a >> 8) & 0xff;
  payload[6] = (value_a >> 16) & 0xff;
  payload[7] = (value_a >> 24) & 0xff;

  payload[8] = (value_t >> 0) & 0xff;
  payload[9] = (value_t >> 8) & 0xff;
  payload[10] = (value_t >> 16) & 0xff;
  payload[11] = (value_t >> 24) & 0xff;

  payload_len = 12;
}

void send_lorawan(void)
{
  Serial.print("Sending LoRaWAN message: ");
  status = sx126x.send_uplink((byte *)payload, payload_len, NULL, NULL);

  if (status == RFT_STATUS_OK)
  {
    Serial.println("receive downlink packet");
    Serial.print("    RSSI: ");
    Serial.println(sx126x.get_rssi());
    Serial.print("    SNR: ");
    Serial.println(sx126x.get_snr());
    Serial.print("    Signal rssi: ");
    Serial.println(sx126x.get_signal_rssi());

    Serial.print("Downlink payload: ");
    for (int i = 0; i < sx126x.get_rx_length(); i++)
    {
      Serial.print(payload[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  else
  {
    Serial.println(rft_status_to_str(status));
  }
}
