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

#define FREQ_STEP_IN_HZ 5000             // Hz
#define CENTER_FREQUENCY_IN_HZ 868100000 // Hz
#define DOPPLER_BANDWIDTH_IN_HZ 100000   // Hz

rfthings_sx126x sx126x(E22_NSS, E22_NRST, E22_BUSY, E22_DIO1, E22_RXEN);
rft_status_t status;

char payload[255];
uint32_t payload_len;

void setup()
{
    Serial.begin(115200);

    while (!Serial && (millis() < 3000))
        ;

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Init SX126x
    Serial.println("#### SX126X INITIALIZE ####");
    status = sx126x.init(RFT_REGION_EU863_870);
    Serial.print("SX126x initialization: ");
    Serial.println(rft_status_to_str(status));

    // LoRa parameters
    sx126x.set_frequency(868100000);
    sx126x.set_frequency(CENTER_FREQUENCY_IN_HZ);
    sx126x.set_spreading_factor(RFT_LORA_SPREADING_FACTOR_9);
    sx126x.set_bandwidth(RFT_LORA_BANDWIDTH_250KHZ);
    sx126x.set_coding_rate(RFT_LORA_CODING_RATE_4_5);
    sx126x.set_syncword(RFT_LORA_SYNCWORD_PUBLIC);
}

void loop()
{
    Serial.print("Listening for LoRa PHY message: ");

    status = sx126x.receive_lora((byte *)payload, payload_len, 2000, NULL);
    Serial.println(rft_status_to_str(status));

    if (status == RFT_STATUS_OK)
    {
        Serial.println("receive downlink packet");
        Serial.print("    RSSI: ");
        Serial.println(sx126x.get_rssi());
        Serial.print("    SNR: ");
        Serial.println(sx126x.get_snr());
        Serial.print("    Signal rssi: ");
        Serial.println(sx126x.get_signal_rssi());

        uint8_t len = sx126x.get_rx_length();

        if (len == 2)
        {
            Serial.print("Downlink payload ");
            Serial.print(len);
            Serial.print(" bytes: ");
            Serial.print(payload[0], HEX);
            Serial.print(" ");
            Serial.println();

            uint16_t offset_index = payload[0] & 0xff;

            int32_t freq_offset = 0;
            freq_offset -= (DOPPLER_BANDWIDTH_IN_HZ / 2);
            freq_offset += (offset_index * FREQ_STEP_IN_HZ);

            Serial.print("Packet received at offset of ");
            Serial.print(freq_offset);
            Serial.println(" Hz\n\n\n");
        }
        else
        {
            Serial.print("Downlink payload ");
            Serial.print(len);
            Serial.print(" bytes: ");
            for (int i = 0; i < len; i++)
            {
                Serial.print(payload[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }

        digitalWrite(LED_BUILTIN, HIGH);
        delay(125);
        digitalWrite(LED_BUILTIN, LOW);
        delay(50);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(125);
        digitalWrite(LED_BUILTIN, LOW);
    }
}
