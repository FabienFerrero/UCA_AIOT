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

#include "../rfthings_radio.h"
#include "lr11xx_driver/lr11xx_hal.h"
#include "lr11xx_driver/lr11xx_gnss.h"
#include "lr11xx_driver/lr11xx_radio.h"
#include "lr11xx_driver/lr11xx_regmem.h"
#include "lr11xx_driver/lr11xx_system.h"
#include "lr11xx_driver/lr11xx_gnss_types.h"
#include "lr11xx_driver/lr11xx_bootloader.h"
#include "lr11xx_driver/lr11xx_radio_types.h"
#include "lr11xx_driver/lr11xx_system_types.h"
#include "lr11xx_driver/lr11xx_bootloader_types.h"
#include "lr11xx_firmware_images/lr11xx_firmware_update_type.h"

class rfthings_lr11xx : public rfthings_radio
{
public:
    rfthings_lr11xx(byte nss_pin, byte rst_pin, byte busy_pin, byte dio1_pin);
    ~rfthings_lr11xx(void);

    rft_status_t init(rft_region_t region);

    rft_status_t send_lora(byte *payload, uint32_t payload_len, void (*tx_func)());
    rft_status_t receive_lora(byte *payload, uint32_t payload_len, void (*rx_func)());
    rft_status_t send_uplink(byte *payload, uint32_t payload_len, void (*tx_func)(), void (*rx_func)());
    rft_status_t send_join_request(void (*tx_func)(), void (*rx_func)());
    rft_status_t check_hardware(void);

    rft_status_t update_firmware(lr11xx_fw_update_t update, const uint32_t *buffer, uint32_t length);

    void config_continous_wave(void);
    void start_continuous_wave(void);
    void stop_continuous_wave(void);
    rft_status_t sweep_continuous_wave(uint32_t start_freq, uint32_t stop_freq, uint32_t step, uint16_t duration);

    void scan_gnss_autonomous(uint8_t nb_sat = 20);
    void get_gnss_satelites(uint8_t *nb_detected_satellites, lr11xx_gnss_detected_satellite_t *detected_satellite_id_snr_doppler);

    void get_nav_message(uint8_t *nav_message, uint16_t *nav_message_len);

    void sleep(void);
    void wake_up(void);

    uint8_t get_system_hardware(void);
    uint8_t get_system_type(void);
    uint16_t get_system_firmware(void);

    uint8_t get_bootloader_hardware(void);
    uint8_t get_bootloader_type(void);
    uint16_t get_bootloader_firmware(void);

    uint8_t get_gnss_firmware(void);
    uint8_t get_gnss_almanac(void);

    uint8_t *get_pin(void);
    uint8_t *get_chip_eui(void);
    uint8_t *get_join_eui(void);

private:
    lr11xx_hal_t lr11xx_hal;
    lr11xx_system_version_t lr11xx_system_version;
    lr11xx_bootloader_version_t lr11xx_bootloader_version;
    lr11xx_gnss_version_t lr11xx_gnss_version;

    lr11xx_bootloader_pin_t pin;
    lr11xx_bootloader_chip_eui_t chip_eui;
    lr11xx_bootloader_join_eui_t join_eui;

    bool lr11xx_is_fw_compatible_with_chip(lr11xx_fw_update_t update, uint16_t bootloader_version);

    static lr11xx_radio_lora_sf_t map_spreading_factor(rft_lora_spreading_factor_t spreading_factor);
    static lr11xx_radio_lora_bw_t map_bandwidth(rft_lora_bandwidth_t bandwidth);
    static lr11xx_radio_lora_cr_t map_coding_rate(rft_lora_coding_rate_t coding_rate);

    uint8_t compute_lora_ldro(void);
    uint8_t compute_lora_ldro(rft_lora_spreading_factor_t _spreading_factor, rft_lora_bandwidth_t _bandwidth);
    lr11xx_radio_rssi_calibration_table_t *get_rssi_calibration_table(void);

    static lr11xx_radio_rssi_calibration_table_t rssi_calibration_table_below_600mhz;
    static lr11xx_radio_rssi_calibration_table_t rssi_calibration_table_from_600mhz_to_2ghz;
    static lr11xx_radio_rssi_calibration_table_t rssi_calibration_table_above_2ghz;
};