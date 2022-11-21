/*
 *     __          ____        _____                       __    _ __
 *    / /   ____  / __ \____ _/ ___/____  ____ _________  / /   (_) /_
 *   / /   / __ \/ /_/ / __ `/\__ \/ __ \/ __ `/ ___/ _ \/ /   / / __ \
 *  / /___/ /_/ / _, _/ /_/ /___/ / /_/ / /_/ / /__/  __/ /___/ / /_/ /
 * /_____/\____/_/ |_|\__,_//____/ .___/\__,_/\___/\___/_____/_/_.___/
 *                              /_/
 * Author: m1nhle, mtnguyen
 * Lib jointy developed by UCA & RFThings
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

#include <RFThings.h>
#include <rfthings_sx126x.h>

#define TX_INTERVAL 15

// Keys and device address are MSB
static uint8_t nwkS_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t appS_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t dev_addr[] = {0x00, 0x00, 0x00, 0x00};

// DKPlatium
rfthings_sx126x sx126x(E22_NSS, E22_NRST, E22_BUSY, E22_DIO1, E22_RXEN);
rft_status_t status;

char payload[255];
uint32_t payload_len;

String message = "Hello world";

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(125);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(125);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  while (!Serial && (millis() < 3000))
    ;
  Serial.println("Sensor-to-Relay example for DKPlatium");

  // Inititialize the LoRa Module SX126x
  status = sx126x.init(RFT_REGION_EU863_870);
  Serial.print("LoRa SX126x: ");
  Serial.println(rft_status_to_str(status));

  // set up LoRaWAN key (ABP by default)
  sx126x.set_lorawan_activation_type(RFT_LORAWAN_ACTIVATION_TYPE_ABP);
  sx126x.set_application_session_key(appS_key);
  sx126x.set_network_session_key(nwkS_key);
  sx126x.set_device_address(dev_addr);

  // Set a custom LoRaWAN configuration
  sx126x.set_send_to_relay(true);
  sx126x.set_frequency(866600000);
  sx126x.set_spreading_factor(RFT_LORA_SPREADING_FACTOR_7);
  sx126x.set_bandwidth(RFT_LORA_BANDWIDTH_125KHZ);
  sx126x.set_coding_rate(RFT_LORA_CODING_RATE_4_5);
  sx126x.set_syncword(RFT_LORA_SYNCWORD_PUBLIC);
  sx126x.set_tx_power(14);

  delay(2000);
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(125);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(125);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.print("Sending LoRaWAN message: ");

  build_payload();
  status = sx126x.send_uplink((byte *)payload, payload_len, NULL, NULL);
  Serial.println(rft_status_to_str(status));

  delay(TX_INTERVAL * 1000);
}

void build_payload(void)
{
  message.toCharArray(payload, 255);
  payload_len = message.length();
}
