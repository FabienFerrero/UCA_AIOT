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

#include "rfthings_radio.h"

rft_status_t rfthings_radio::create_params_by_region(rft_region_t region) {
    lora_params.coding_rate = RFT_LORA_CODING_RATE_4_5;
    lora_params.syncword = RFT_LORA_SYNCWORD_PUBLIC;

    lora_params.snr = 0;
    lora_params.rssi = 0;
    lora_params.signal_rssi = 0;
    
    lora_params.send_to_relay = false;
	lora_params.relay_sleep_interval_us = 1E6; // 1 second
	lora_params.relay_rx_symbol = 5; // Default value is 5 symbols
	lora_params.relay_max_rx_packet_length = 120; // Default value is 120 bytes

    lorawan_params.activation_type = RFT_LORAWAN_ACTIVATION_TYPE_ABP;

    memset(lorawan_params.device_address, 0x00, 8);
    memset(lorawan_params.network_session_key, 0x00, 16);
    memset(lorawan_params.application_session_key, 0x00, 16);
    memset(lorawan_params.devive_eui, 0x00, 8);
    memset(lorawan_params.application_eui, 0x00, 16);
    memset(lorawan_params.application_key, 0x00, 16);
    memset(lorawan_params.dev_nonce, 0x00, 2);

    lorawan_params.rx1_delay = 1000;
    lorawan_params.framecounter_size = RFT_LORAWAN_FRAMECOUNTER_SIZE_32;
    lorawan_params.framecounter_uplink = 0;
    lorawan_params.framecounter_downlink = 0;
    lorawan_params.tx_port = 1;
    lorawan_params.rx_port = 0;
    lorawan_params.rx_length = 0;

    // LR-FHSS application
    lrfhss_params.codingRate = RFT_LRFHSS_CODING_RATE_1_3;
    lrfhss_params.bandwidth = RFT_LRFHSS_BANDWIDTH_335_9_KHZ;
    lrfhss_params.hopping = true;
    lrfhss_params.nbSync = 4;
    lrfhss_params.power = 21;
    lrfhss_params.grid = RFT_LRFHSS_GRID_3_9_KHZ;

    if (region == RFT_REGION_EU863_870) {
        lora_params.tx_power = 16;
        lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
        lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lora_params.frequency = 868100000;

        lorawan_params.rx1_frequency = 868100000;
        lorawan_params.rx2_frequency = 869525000;
        lorawan_params.rx2_bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lorawan_params.rx2_spreading_factor = RFT_LORA_SPREADING_FACTOR_9;

        // LR-FHSS application
        lrfhss_params.frequency = 862750000;
    } else if (region == RFT_REGION_US902_928) {
        lora_params.tx_power = 18;
        lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
        lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lora_params.frequency = 903100000;

        lorawan_params.rx1_frequency = 923300000;
        lorawan_params.rx2_frequency = 923300000;
        lorawan_params.rx2_bandwidth = RFT_LORA_BANDWIDTH_500KHZ;
        lorawan_params.rx2_spreading_factor = RFT_LORA_SPREADING_FACTOR_12;

        // LR-FHSS application
        lrfhss_params.frequency = 924000000;
    } else if (region == RFT_REGION_CN470_510) {
        lora_params.tx_power = 18;
        lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
        lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lora_params.frequency = 486300000;

        lorawan_params.rx1_frequency = 506700000;
        lorawan_params.rx2_frequency = 505300000;
        lorawan_params.rx2_bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lorawan_params.rx2_spreading_factor = RFT_LORA_SPREADING_FACTOR_12;

        // LR-FHSS application
        lrfhss_params.frequency = 862750000;
    } else if (region == RFT_REGION_AU915_928) {
        lora_params.tx_power = 18;
        lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
        lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lora_params.frequency = 916800000;

        lorawan_params.rx1_frequency = 923300000;
        lorawan_params.rx2_frequency = 923300000;
        lorawan_params.rx2_bandwidth = RFT_LORA_BANDWIDTH_500KHZ;
        lorawan_params.rx2_spreading_factor = RFT_LORA_SPREADING_FACTOR_12;

        // LR-FHSS application
        lrfhss_params.frequency = 924000000;
    } else if (region == RFT_REGION_AS920_923) {
        lora_params.tx_power = 18;
        lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
        lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lora_params.frequency = 923200000;

        lorawan_params.rx1_frequency = 923200000;
        lorawan_params.rx2_frequency = 923200000;
        lorawan_params.rx2_bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lorawan_params.rx2_spreading_factor = RFT_LORA_SPREADING_FACTOR_10;

        // LR-FHSS application
        lrfhss_params.frequency = 924000000;
    } else if (region == RFT_REGION_AS923_925) {
        lora_params.tx_power = 19;
        lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
        lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lora_params.frequency = 923200000;

        lorawan_params.rx1_frequency = 924000000;
        lorawan_params.rx2_frequency = 923200000;
        lorawan_params.rx2_bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lorawan_params.rx2_spreading_factor = RFT_LORA_SPREADING_FACTOR_10;

        // LR-FHSS application
        lrfhss_params.frequency = 924000000;
    } else if (region == RFT_REGION_KR920_923) {
        lora_params.tx_power = 18;
        lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
        lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lora_params.frequency = 922100000;

        lorawan_params.rx1_frequency = 922100000;
        lorawan_params.rx2_frequency = 922900000;
        lorawan_params.rx2_bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lorawan_params.rx2_spreading_factor = RFT_LORA_SPREADING_FACTOR_12;

        // LR-FHSS application
        lrfhss_params.frequency = 862750000;
    } else if (region == RFT_REGION_IN865_867) {
        lora_params.tx_power = 18;
        lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
        lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lora_params.frequency = 865062500;

        lorawan_params.rx1_frequency = 865062500;
        lorawan_params.rx2_frequency = 866550000;
        lorawan_params.rx2_bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
        lorawan_params.rx2_spreading_factor = RFT_LORA_SPREADING_FACTOR_10;

        // LR-FHSS application
        lrfhss_params.frequency = 866250000;
    } else {
        return RFT_STATUS_ERROR_INVALID_REGION;
    }

    return RFT_STATUS_OK;
}

void rfthings_radio::set_tx_power(int8_t tx_power) {
    lora_params.tx_power = tx_power;
}

void rfthings_radio::set_spreading_factor(rft_lora_spreading_factor_t spreading_factor) {
    lora_params.spreading_factor = spreading_factor;
}

void rfthings_radio::set_coding_rate(rft_lora_coding_rate_t coding_rate) { 
    lora_params.coding_rate = coding_rate;
}

void rfthings_radio::set_bandwidth(rft_lora_bandwidth_t bandwidth) {
    lora_params.bandwidth = bandwidth;
}

void rfthings_radio::set_syncword(rft_lora_syncword_t syncword) {
    lora_params.syncword = syncword;
}

void rfthings_radio::set_frequency(uint32_t frequency) {
    lora_params.frequency = frequency;
}

int8_t rfthings_radio:: get_tx_power(void) {
    return lora_params.tx_power; 
}

rft_lora_spreading_factor_t rfthings_radio:: get_spreading_factor(void) {
    return lora_params.spreading_factor;
}

rft_lora_coding_rate_t rfthings_radio:: get_coding_rate(void) {
    return lora_params.coding_rate;
}

rft_lora_bandwidth_t rfthings_radio:: get_bandwidth(void) {
    return lora_params.bandwidth;
}

rft_lora_syncword_t rfthings_radio:: get_syncword(void) {
    return lora_params.syncword;
}

uint32_t rfthings_radio:: get_frequency(void) {
    return lora_params.frequency;
}

int8_t rfthings_radio::get_snr(void) {
    return lora_params.snr;
}

int16_t rfthings_radio::get_rssi(void) {
    return lora_params.rssi;
}

int16_t rfthings_radio::get_signal_rssi(void) {
    return lora_params.signal_rssi;
}

bool rfthings_radio::get_send_to_relay(void)
{
    return lora_params.send_to_relay;
}

uint32_t rfthings_radio::get_relay_sleep_interval_us(void)
{
    return lora_params.relay_sleep_interval_us;
}

uint8_t rfthings_radio::get_relay_rx_symbol(void)
{
    return lora_params.relay_rx_symbol;
}

uint8_t rfthings_radio::get_relay_max_rx_packet_length(void)
{
    return lora_params.relay_max_rx_packet_length;
}

void rfthings_radio::set_send_to_relay(bool _send_to_relay)
{
    lora_params.send_to_relay = _send_to_relay;
}

void rfthings_radio::set_relay_sleep_interval_us(uint32_t _relay_sleep_interval_us)
{
    lora_params.relay_sleep_interval_us = _relay_sleep_interval_us;
}

void rfthings_radio::set_relay_rx_symbol(uint8_t _relay_rx_symbol)
{
    lora_params.relay_rx_symbol = _relay_rx_symbol;
}

void rfthings_radio::set_relay_max_rx_packet_length(uint8_t _relay_max_rx_packet_length)
{
    lora_params.relay_max_rx_packet_length = _relay_max_rx_packet_length;
}

void rfthings_radio::set_lorawan_activation_type(rft_lorawan_activation_type_t activation_type) {
    lorawan_params.activation_type = activation_type;
}

void rfthings_radio::set_device_address(uint8_t dev_addr[4]) {
    memcpy(lorawan_params.device_address, dev_addr, 4);
}

void rfthings_radio::set_network_session_key(uint8_t nwkS_key[16]) {
    memcpy(lorawan_params.network_session_key, nwkS_key, 16);
}
void rfthings_radio::set_application_session_key(uint8_t appS_key[16]) {
    memcpy(lorawan_params.application_session_key, appS_key, 16);
}

void rfthings_radio::set_devive_eui(uint8_t dev_eui[8]) {
    memcpy(lorawan_params.devive_eui, dev_eui, 8);
}

void rfthings_radio::set_application_eui(uint8_t app_eui[8]) {
    memcpy(lorawan_params.application_eui, app_eui, 8);
}

void rfthings_radio::set_application_key(uint8_t app_key[16]) {
    memcpy(lorawan_params.application_key, app_key, 16);
}

void rfthings_radio::set_framecounter_size(rft_lorawan_framecounter_size_t framecounter_size) {
    lorawan_params.framecounter_size = framecounter_size;
}

void rfthings_radio::set_tx_port(uint8_t tx_port) {
    lorawan_params.tx_port = tx_port;
}

void rfthings_radio::set_rx1_delay(uint32_t rx1_delay) {
    lorawan_params.rx1_delay = rx1_delay;
}

rft_lorawan_activation_type_t rfthings_radio::get_lorawan_activation_type(void) {
    return lorawan_params.activation_type;
}

uint8_t* rfthings_radio::get_device_address(void) {
    return lorawan_params.device_address;
}

uint8_t* rfthings_radio::get_network_session_key(void) {
    return lorawan_params.network_session_key;
}

uint8_t* rfthings_radio::get_application_session_key(void) {
    return lorawan_params.application_session_key;
}

uint8_t* rfthings_radio::get_devive_eui(void) {
    return lorawan_params.devive_eui;
}

uint8_t* rfthings_radio::get_application_eui(void) {
    return lorawan_params.application_eui;
}

uint8_t* rfthings_radio::get_application_key(void) {
    return lorawan_params.application_key;
}

rft_lorawan_framecounter_size_t rfthings_radio::get_framecounter_size(void) {
    return lorawan_params.framecounter_size;
}
uint8_t rfthings_radio::get_tx_port(void) {
    return lorawan_params.tx_port;
}

uint32_t rfthings_radio::get_framecounter_uplink(void) {
    return lorawan_params.framecounter_uplink;
}

uint32_t rfthings_radio::get_framecounter_downlink(void) {
    return lorawan_params.framecounter_downlink;
}

uint8_t rfthings_radio::get_rx_port(void) {
    return lorawan_params.rx_port;
}

uint8_t rfthings_radio::get_rx_length(void) {
    return lorawan_params.rx_length;
}

void rfthings_radio::set_lrfhss_codingRate(rft_lrfhss_coding_rate_t codingRate)
{
    lrfhss_params.codingRate = codingRate;
}

void rfthings_radio::set_lrfhss_bandwidth(rft_lrfhss_bandwidth_t bandwidth)
{
    lrfhss_params.bandwidth = bandwidth;
}

void rfthings_radio::set_lrfhss_grid(rft_lrfhss_grid_t grid)
{
    lrfhss_params.grid = grid;
}

void rfthings_radio::set_lrfhss_hopping(bool hopping)
{
    lrfhss_params.hopping = hopping;
}

void rfthings_radio::set_lrfhss_nbSync(uint8_t nbSync)
{
    lrfhss_params.nbSync = nbSync;
}

void rfthings_radio::set_lrfhss_frequency(uint32_t frequency)
{
    lrfhss_params.frequency = frequency;
}

void rfthings_radio::set_lrfhss_power(int8_t power)
{
    lrfhss_params.power = power;
}

rft_lrfhss_coding_rate_t rfthings_radio::get_lrfhss_codingRate(void)
{
    return lrfhss_params.codingRate;
}

rft_lrfhss_bandwidth_t rfthings_radio::get_lrfhss_bandwidth(void)
{
    return lrfhss_params.bandwidth;
}

rft_lrfhss_grid_t rfthings_radio::get_lrfhss_grid(void)
{
    return lrfhss_params.grid;
}

bool rfthings_radio::get_lrfhss_hopping(void)
{
    return lrfhss_params.hopping;
}

uint8_t rfthings_radio::get_lrfhss_nbSync(void)
{
    return lrfhss_params.nbSync;
}

uint32_t rfthings_radio::get_lrfhss_frequency(void)
{
    return lrfhss_params.frequency;
}

int8_t rfthings_radio::get_lrfhss_power(void)
{
    return lrfhss_params.power;
}


uint8_t rfthings_radio::build_uplink_packet(unsigned char *payload, uint8_t payload_len, unsigned char *lorawan_packet) {
    uint8_t packet_len = 0;

	unsigned char MAC_header = 0x40; // Unconfirmed data up
	unsigned char FCtrl = 0x00;
    unsigned char MIC[4];

    lorawan_packet[0] = MAC_header;
    lorawan_packet[1] = lorawan_params.device_address[3];
    lorawan_packet[2] = lorawan_params.device_address[2];
    lorawan_packet[3] = lorawan_params.device_address[1];
    lorawan_packet[4] = lorawan_params.device_address[0];
    lorawan_packet[5] = FCtrl;

    lorawan_packet[6] = (lorawan_params.framecounter_uplink & 0x00FF);
    lorawan_packet[7] = ((lorawan_params.framecounter_uplink >> 8) & 0x00FF);
    lorawan_packet[8] = lorawan_params.tx_port;

    packet_len = 9;

    Encrypt_Payload(payload, payload_len, lorawan_params.application_session_key, lorawan_params.device_address, 0x00, lorawan_params.framecounter_uplink);

    for (int i = 0; i < payload_len; i++) {
        lorawan_packet[packet_len++] = payload[i];
    }

    Construct_Data_MIC(lorawan_packet, packet_len, lorawan_params.network_session_key, lorawan_params.device_address, 0x00, lorawan_params.framecounter_uplink, MIC);

    for (int i = 0; i<4; i++) {
        lorawan_packet[packet_len++] = MIC[i];
    }

    return packet_len;
}

uint8_t rfthings_radio::build_join_request(unsigned char *lorawan_packet) {    
    uint8_t packet_len = 0;

	unsigned char MAC_header = 0x00; // Join request
    unsigned char MIC[4];

    lorawan_packet[0] = MAC_header;
    packet_len = 1;

    for (int i = 0; i < 8; i++) {
        lorawan_packet[packet_len++] = lorawan_params.application_eui[7-i];
    }
    
    for (int i = 0; i < 8; i++) {
        lorawan_packet[packet_len++] = lorawan_params.devive_eui[7-i];
    }

    unsigned int dev_nonce = random(0xffff);
    lorawan_packet[17] = lorawan_params.dev_nonce[0] = dev_nonce & 0x00ff;
    lorawan_packet[18] = lorawan_params.dev_nonce[1] = (dev_nonce >> 8) & 0x00ff;
    packet_len = 19;

    Calculate_MIC(lorawan_packet, packet_len, lorawan_params.application_key, MIC);

    for (int i = 0; i < 4; i++) {
        lorawan_packet[packet_len++] = MIC[i];
    }

    return packet_len;
}

rft_status_t rfthings_radio::parse_downlink(unsigned char *payload, uint8_t payload_len) {
    unsigned char MAC_header = payload[0];

    if (MAC_header == 0x40 || MAC_header == 0x60 || MAC_header == 0x80 || MAC_header == 0xa0) {
        unsigned char dev_addr[4];
        unsigned char FCtrl;
        unsigned char MIC[4];

        dev_addr[0] = payload[4];
        dev_addr[1] = payload[3];
        dev_addr[2] = payload[2];
        dev_addr[3] = payload[1];

        FCtrl = payload[5];
        lorawan_params.framecounter_downlink = payload[7];
        lorawan_params.framecounter_downlink = (lorawan_params.framecounter_downlink << 8) + payload[6];

        lorawan_params.rx_port = payload[8];

        Construct_Data_MIC(payload, payload_len-4, lorawan_params.network_session_key, dev_addr, 0x01, lorawan_params.framecounter_downlink, MIC);

        // Check MIC
        for (int i = 0; i < 4; i++) {
            if (payload[payload_len - 4 + i] != MIC[i]) {
                return RFT_STATUS_WRONG_MIC;
            }
        }

        memmove(payload, payload + 9, payload_len - 9 - 4);
        payload_len = payload_len - 9 - 4;

        if (lorawan_params.rx_port == 0x00) {
            Encrypt_Payload(payload, payload_len, lorawan_params.network_session_key, lorawan_params.device_address, 0x01, lorawan_params.framecounter_downlink);
        } else {
            Encrypt_Payload(payload, payload_len, lorawan_params.network_session_key, lorawan_params.application_session_key, 0x01, lorawan_params.framecounter_downlink);
        }

        lorawan_params.rx_length = payload_len;

        return RFT_STATUS_OK;
    }

    return RFT_STATUS_RX_TIMEOUT;
}

rft_status_t rfthings_radio::parse_join_accept(unsigned char *payload, uint8_t payload_len) {
    unsigned char MAC_header = payload[0];

    if (MAC_header == 0x20) {
        unsigned char MIC[4];

        for(int i = 0; i < ((payload_len - 1) / 16); i++) {
            AES_Encrypt(payload + i * 16 + 1, lorawan_params.application_key);
        }

        Calculate_MIC(payload, payload_len - 4, lorawan_params.application_key, MIC);

        // Check MIC
        for (int i = 0; i < 4; i++) {
            if (payload[payload_len - 4 + i] != MIC[i]) {
                return RFT_STATUS_WRONG_MIC;
            }
        }

        // device address
        for (int i = 0; i < 4; i++) {
            lorawan_params.device_address[i] = payload[10 - i];
        }

        // NwkSKey = aes128_encrypt(AppKey, 0x01|AppNonce|NetID|DevNonce|pad16)
        // AppSKey = aes128_encrypt(AppKey, 0x02|AppNonce|NetID|DevNonce|pad16)
        lorawan_params.network_session_key[0] = 0x01;
        lorawan_params.application_session_key[0] = 0x02;

        // app nonce
        for (int i = 0; i < 3; i++) {
            lorawan_params.network_session_key[i + 1] = payload[i + 1];
            lorawan_params.application_session_key[i + 1] = payload[i + 1];
        }

        // net ID
        for (int i = 0; i < 3; i++) {
            lorawan_params.network_session_key[i + 4] = payload[i + 4];
            lorawan_params.application_session_key[i + 4] = payload[i + 4];
        }

        lorawan_params.network_session_key[7] = lorawan_params.application_session_key[7] = lorawan_params.dev_nonce[0];
        lorawan_params.network_session_key[8] = lorawan_params.application_session_key[8] = lorawan_params.dev_nonce[1];

        memset(lorawan_params.network_session_key + 9, 0x00, 7);
        memset(lorawan_params.application_session_key + 9, 0x00, 7);

        AES_Encrypt(lorawan_params.network_session_key, lorawan_params.application_key);
        AES_Encrypt(lorawan_params.application_session_key, lorawan_params.application_key);

        lorawan_params.framecounter_uplink = 0;
        lorawan_params.rx_length = 0;

        return RFT_STATUS_OK;
    }

    return RFT_STATUS_RX_TIMEOUT;
}
