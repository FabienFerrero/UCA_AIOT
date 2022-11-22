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
#define TX_INTERVAL 1                    // Seconds

rfthings_sx126x sx126x(E22_NSS, E22_NRST, E22_BUSY, E22_DIO1, E22_RXEN);
rft_status_t status;

char payload[255];
uint32_t payload_len;

uint32_t current_freq_offset = 0;

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
    sx126x.set_tx_power(22);
    sx126x.set_force_ldro(true);
    sx126x.set_frequency(CENTER_FREQUENCY_IN_HZ);
    sx126x.set_spreading_factor(RFT_LORA_SPREADING_FACTOR_9);
    sx126x.set_bandwidth(RFT_LORA_BANDWIDTH_250KHZ);
    sx126x.set_coding_rate(RFT_LORA_CODING_RATE_4_5);
    sx126x.set_syncword(RFT_LORA_SYNCWORD_PUBLIC);

    current_freq_offset = 0;
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

    Serial.print("Sending LoRa PHY message: ");

    recalculate_frequency();
    build_payload();
    status = sx126x.send_lora((byte *)payload, payload_len, 2000, NULL);
    Serial.println(rft_status_to_str(status));

    delay(TX_INTERVAL * 1000);
}

void build_payload(void)
{
    payload[0] = 0xff & (current_freq_offset >> 0);
    payload[1] = 0xff & (current_freq_offset >> 8);

    payload_len = 4;
}

void recalculate_frequency(void)
{
    if ((current_freq_offset * FREQ_STEP_IN_HZ) > DOPPLER_BANDWIDTH_IN_HZ)
    {
        current_freq_offset = 0;
    }

    uint32_t expected_frequency = CENTER_FREQUENCY_IN_HZ;
    expected_frequency -= (DOPPLER_BANDWIDTH_IN_HZ / 2);
    expected_frequency += (current_freq_offset * FREQ_STEP_IN_HZ);
    sx126x.set_frequency(expected_frequency);

    Serial.print("> Step ");
    Serial.print(current_freq_offset);
    Serial.println(" :");

    Serial.print("          Expected frequency: ");
    Serial.print(expected_frequency);
    Serial.println(" Hz");

    // Estimate the actual frequency due to hz_to_pll_step conversion
    double actual_frequency = sx126x_convert_freq_in_hz_to_pll_step(expected_frequency);
    actual_frequency *= (32000000UL); // SX126X_XTAL_FREQ;
    actual_frequency /= (1 << 25);


    Serial.print("          Actual frequency:   ");
    Serial.print(actual_frequency);
    Serial.println(" Hz");

    current_freq_offset++;
}
