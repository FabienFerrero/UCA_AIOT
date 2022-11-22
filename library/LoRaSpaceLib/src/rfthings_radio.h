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

#include "Arduino.h"
#include "SPI.h"
#include "RFThings.h"

class rfthings_radio {
    public:
        // rfthings_radio(void);
        // ~rfthings_radio();
        
        // virtual rft_status_t init(void) = 0;
        
        // virtual rft_status_t send_lora(unsigned char *payload, uint8_t payload_len) = 0;
        // virtual rft_status_t send_lorawan(unsigned char *payload, uint8_t payload_len) = 0;
        // virtual rft_status_t check_hardware(void) = 0;

        // virtual void start_continuous_wave(void) = 0;
        // virtual void stop_continuous_wave(void) = 0;
        // virtual rft_status_t sweep_continuous_wave(uint32_t start_freq, uint32_t stop_freq, uint32_t step, uint16_t duration) = 0;
        
        // virtual void wait_for_busy(void) = 0;
        // virtual void sleep(void) = 0;
        // virtual void wake_up(void) = 0;

        // set LoRa tx parameters
        void set_tx_power(int8_t tx_power);
        void set_spreading_factor(rft_lora_spreading_factor_t spreading_factor);
        void set_coding_rate(rft_lora_coding_rate_t coding_rate);
        void set_bandwidth(rft_lora_bandwidth_t bandwidth);
        void set_syncword(rft_lora_syncword_t syncword);
        void set_frequency(uint32_t frequency);

        // get LoRa tx parameters
        int8_t get_tx_power(void);
        rft_lora_spreading_factor_t get_spreading_factor(void);
        rft_lora_coding_rate_t get_coding_rate(void);
        rft_lora_bandwidth_t get_bandwidth(void);
        rft_lora_syncword_t get_syncword(void);
        uint32_t get_frequency(void);
        
        // get LoRa rx parameters
        int8_t get_snr(void);
        int16_t get_rssi(void);
        int16_t get_signal_rssi(void);

        // LoRaWAN relay application
        bool get_send_to_relay(void);
        uint32_t get_relay_sleep_interval_us(void);
        uint8_t get_relay_rx_symbol(void);
        uint8_t get_relay_max_rx_packet_length(void);

        void set_send_to_relay(bool _send_to_relay);
        void set_relay_sleep_interval_us(uint32_t _relay_sleep_interval_us);
        void set_relay_rx_symbol(uint8_t _relay_rx_symbol);
        void set_relay_max_rx_packet_length(uint8_t _relay_max_rx_packet_length);

        // set LoRaWAN params
        void set_lorawan_activation_type(rft_lorawan_activation_type_t activation_type);
        void set_device_address(uint8_t dev_addr[4]);
        void set_network_session_key(uint8_t nwkS_key[16]);
        void set_application_session_key(uint8_t appS_key[16]);

        void set_devive_eui(uint8_t dev_eui[8]);
        void set_application_eui(uint8_t app_eui[8]);
        void set_application_key(uint8_t app_key[16]);

        void set_framecounter_size(rft_lorawan_framecounter_size_t framecounter_size);
        void set_tx_port(uint8_t tx_port);
        void set_rx1_delay(uint32_t rx1_delay);

        // get LoRaWAN params
        rft_lorawan_activation_type_t get_lorawan_activation_type(void);
        uint8_t* get_device_address(void);
        uint8_t* get_network_session_key(void);
        uint8_t* get_application_session_key(void);

        uint8_t* get_devive_eui(void);
        uint8_t* get_application_eui(void);
        uint8_t* get_application_key(void);

        rft_lorawan_framecounter_size_t get_framecounter_size(void);
        uint8_t get_tx_port(void);

        // get LoRaWAN params
        uint32_t get_framecounter_uplink(void);
        uint32_t get_framecounter_downlink(void);
        uint8_t get_rx_port(void);
        uint8_t get_rx_length(void);

        // set LR-FHSS params
        void set_lrfhss_codingRate(rft_lrfhss_coding_rate_t codingRate);
        void set_lrfhss_bandwidth(rft_lrfhss_bandwidth_t bandwidth);
        void set_lrfhss_grid(rft_lrfhss_grid_t grid);
        void set_lrfhss_hopping(bool hopping);
        void set_lrfhss_nbSync(uint8_t nbSync);
        void set_lrfhss_frequency(uint32_t frequency);
        void set_lrfhss_power(int8_t power);

        // get LR-FHSS params
        rft_lrfhss_coding_rate_t get_lrfhss_codingRate(void);
        rft_lrfhss_bandwidth_t get_lrfhss_bandwidth(void);
        rft_lrfhss_grid_t get_lrfhss_grid(void);
        bool get_lrfhss_hopping(void);
        uint8_t get_lrfhss_nbSync(void);
        uint32_t get_lrfhss_frequency(void);
        int8_t get_lrfhss_power(void);

        // For doppler test
        bool get_force_ldro(void);
        void set_force_ldro(bool force_ldro);
        
    protected:
        rft_lora_params_t lora_params;
        rft_lorawan_params_t lorawan_params;
        rft_lrfhss_params_t lrfhss_params;

        rft_status_t create_params_by_region(rft_region_t region);
        uint8_t build_uplink_packet(unsigned char *payload, uint8_t payload_len, unsigned char *lorawan_packet);
        uint8_t build_join_request(unsigned char *lorawan_packet);

        rft_status_t parse_downlink(unsigned char *payload, uint8_t payload_len);
        rft_status_t parse_join_accept(unsigned char *payload, uint8_t payload_len);
};