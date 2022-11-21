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

#include "rfthings_lr11xx.h"

rfthings_lr11xx::rfthings_lr11xx(byte nss_pin, byte rst_pin, byte busy_pin, byte dio1_pin)
{
	lr11xx_hal.nss = nss_pin;
	lr11xx_hal.reset = rst_pin;
	lr11xx_hal.busy = busy_pin;
	lr11xx_hal.irq = dio1_pin;
}

rfthings_lr11xx::~rfthings_lr11xx(void)
{
}

void rfthings_lr11xx::sleep(void)
{
	lr11xx_system_sleep_cfg_t sleep_config;
	sleep_config.is_warm_start = true;
	sleep_config.is_rtc_timeout = false;
	lr11xx_system_set_sleep(&lr11xx_hal, sleep_config, 0);
}

void rfthings_lr11xx::wake_up(void)
{
	lr11xx_hal_wakeup(&lr11xx_hal);
}

rft_status_t rfthings_lr11xx::init(rft_region_t region)
{
	if (create_params_by_region(region) != RFT_STATUS_OK)
	{
		return RFT_STATUS_ERROR_INVALID_REGION;
	}

	pinMode(lr11xx_hal.nss, OUTPUT);
	pinMode(lr11xx_hal.reset, OUTPUT);
	pinMode(lr11xx_hal.busy, INPUT);
	pinMode(lr11xx_hal.irq, INPUT_PULLDOWN);

	memset(pin, 0x00, 4);
	memset(chip_eui, 0x00, 8);
	memset(join_eui, 0x00, 8);

	SPI.begin();
	digitalWrite(lr11xx_hal.nss, HIGH);

	lr11xx_hal_reset(&lr11xx_hal);

	// Configure the regulator
	lr11xx_system_set_reg_mode(&lr11xx_hal, LR11XX_SYSTEM_REG_MODE_DCDC);

	// Configure the 32MHz TCXO if it is available on the board
	lr11xx_system_set_tcxo_mode(&lr11xx_hal, LR11XX_SYSTEM_TCXO_CTRL_2_7V, 500);

	// Configure the Low Frequency Clock
	lr11xx_system_cfg_lfclk(&lr11xx_hal, LR11XX_SYSTEM_LFCLK_XTAL, true);

	// rf switch
	lr11xx_system_rfswitch_cfg_t rf_switch_setup;
	rf_switch_setup.enable = LR11XX_SYSTEM_RFSW0_HIGH | LR11XX_SYSTEM_RFSW1_HIGH | LR11XX_SYSTEM_RFSW2_HIGH;
	rf_switch_setup.standby = 0;
	rf_switch_setup.rx = LR11XX_SYSTEM_RFSW0_HIGH;
	rf_switch_setup.tx = LR11XX_SYSTEM_RFSW1_HIGH;
	rf_switch_setup.tx_hp = LR11XX_SYSTEM_RFSW1_HIGH;
	rf_switch_setup.tx_hf = 0x00;
	// rf_switch_setup.gnss = LR11XX_SYSTEM_RFSW2_HIGH | LR11XX_SYSTEM_RFSW3_HIGH;
	// rf_switch_setup.wifi 	= LR11XX_SYSTEM_RFSW3_HIGH;
	lr11xx_system_set_dio_as_rf_switch(&lr11xx_hal, &rf_switch_setup);

	lr11xx_system_clear_errors(&lr11xx_hal);

	lr11xx_system_calibrate(&lr11xx_hal, 0x3f);

	if (region == RFT_REGION_EU863_870 || region == RFT_REGION_IN865_867)
	{
		lr11xx_system_calibrate_image(&lr11xx_hal, 0xD7, 0xD8);
	}
	else if (region == RFT_REGION_US902_928 || region == RFT_REGION_AU915_928 || region == RFT_REGION_AS920_923 || region == RFT_REGION_AS923_925 || region == RFT_REGION_KR920_923)
	{
		lr11xx_system_calibrate_image(&lr11xx_hal, 0xE1, 0xE9);
	}
	else if (region == RFT_REGION_CN470_510)
	{
		lr11xx_system_calibrate_image(&lr11xx_hal, 0x75, 0x81);
	}

	uint16_t errors;
	lr11xx_system_get_errors(&lr11xx_hal, &errors);
	lr11xx_system_clear_errors(&lr11xx_hal);
	lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

	lr11xx_system_set_dio_irq_params(&lr11xx_hal, 0x00, 0x00);

	if (check_hardware() != RFT_STATUS_OK)
	{
		return RFT_STATUS_ERROR_HARDWARE;
	}

	lr11xx_radio_set_lora_sync_word(&lr11xx_hal, lora_params.syncword);
	lr11xx_radio_set_lora_public_network(&lr11xx_hal, LR11XX_RADIO_LORA_NETWORK_PUBLIC);

	//&lr11xx_hal init
	lr11xx_radio_set_rssi_calibration(&lr11xx_hal, get_rssi_calibration_table());

	lr11xx_radio_set_rx_tx_fallback_mode(&lr11xx_hal, LR11XX_RADIO_FALLBACK_STDBY_RC);
	lr11xx_radio_cfg_rx_boosted(&lr11xx_hal, true);

	lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

	// GNSS init
	lr11xx_gnss_read_firmware_version(&lr11xx_hal, &lr11xx_gnss_version);
	lr11xx_gnss_set_constellations_to_use(&lr11xx_hal, LR11XX_GNSS_BEIDOU_MASK | LR11XX_GNSS_GPS_MASK);

	// Wifi init

	lr11xx_system_set_standby(&lr11xx_hal, LR11XX_SYSTEM_STANDBY_CFG_RC);

	return RFT_STATUS_OK;
}

rft_status_t rfthings_lr11xx::send_lora(byte *payload, uint32_t payload_len, void (*tx_func)())
{
	lr11xx_hal_wakeup(&lr11xx_hal);
	lr11xx_system_set_standby(&lr11xx_hal, LR11XX_SYSTEM_STANDBY_CFG_RC);

	uint32_t irq_status;
	lr11xx_system_stat1_t stat1;
	lr11xx_system_stat2_t stat2;
	lr11xx_system_clear_errors(&lr11xx_hal);

	lr11xx_radio_set_pkt_type(&lr11xx_hal, LR11XX_RADIO_PKT_TYPE_LORA);

	lr11xx_radio_mod_params_lora_t lora_mod_params;
	lora_mod_params.sf = map_spreading_factor(lora_params.spreading_factor);
	lora_mod_params.bw = map_bandwidth(lora_params.bandwidth);
	lora_mod_params.cr = map_coding_rate(lora_params.coding_rate);
	lora_mod_params.ldro = compute_lora_ldro();
	lr11xx_radio_set_lora_mod_params(&lr11xx_hal, &lora_mod_params);

	lr11xx_radio_pkt_params_lora_t lora_pkt_params;
	lora_pkt_params.preamble_len_in_symb = 8;
	lora_pkt_params.header_type = LR11XX_RADIO_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = payload_len;
	lora_pkt_params.crc = LR11XX_RADIO_LORA_CRC_ON;
	lora_pkt_params.iq = LR11XX_RADIO_LORA_IQ_STANDARD;
	lr11xx_radio_set_lora_pkt_params(&lr11xx_hal, &lora_pkt_params);

	lr11xx_radio_pa_cfg_t pa_config;
	pa_config.pa_sel = LR11XX_RADIO_PA_SEL_HP;
	pa_config.pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT;
	pa_config.pa_duty_cycle = 0x04;
	pa_config.pa_hp_sel = 0x07;
	lr11xx_radio_set_pa_cfg(&lr11xx_hal, &pa_config);

	lr11xx_radio_set_tx_params(&lr11xx_hal, lora_params.tx_power, LR11XX_RADIO_RAMP_48_US);
	lr11xx_radio_set_rf_freq(&lr11xx_hal, lora_params.frequency);

	lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

	lr11xx_regmem_write_buffer8(&lr11xx_hal, payload, payload_len);

	if (tx_func != NULL)
	{
		tx_func();
	}
	lr11xx_radio_set_tx(&lr11xx_hal, 5000);

	while (1)
	{
		lr11xx_system_get_status(&lr11xx_hal, &stat1, &stat2, &irq_status);
		if (irq_status & LR11XX_SYSTEM_IRQ_TX_DONE)
		{
			break;
		}
		if (irq_status & LR11XX_SYSTEM_IRQ_TIMEOUT)
		{
			return RFT_STATUS_TX_TIMEOUT;
		}
	}

	lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

	return RFT_STATUS_TX_DONE;
}

rft_status_t rfthings_lr11xx::receive_lora(byte *payload, uint32_t payload_len, void (*rx_func)())
{
	return RFT_STATUS_OK;
}

rft_status_t rfthings_lr11xx::send_uplink(byte *payload, uint32_t payload_len, void (*tx_func)(), void (*rx_func)())
{
	// buld LoRaWAN packet
	unsigned char lorawan_packet[9 + 255 + 4];

	uint8_t packet_len = build_uplink_packet(payload, payload_len, lorawan_packet);

	// send LoRa
	if (send_lora(lorawan_packet, packet_len, tx_func) == RFT_STATUS_TX_TIMEOUT)
	{
		return RFT_STATUS_TX_TIMEOUT;
	}

	lorawan_params.framecounter_uplink++;

	// receive downlink
	bool receive_downlink = false;

	uint32_t irq_status;
	lr11xx_system_stat1_t stat1;
	lr11xx_system_stat2_t stat2;

	// RX1 window
	lr11xx_radio_pkt_params_lora_t lora_pkt_params;
	lora_pkt_params.preamble_len_in_symb = 8;
	lora_pkt_params.header_type = LR11XX_RADIO_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = 255;
	lora_pkt_params.crc = LR11XX_RADIO_LORA_CRC_ON;
	lora_pkt_params.iq = LR11XX_RADIO_LORA_IQ_INVERTED;
	lr11xx_radio_set_lora_pkt_params(&lr11xx_hal, &lora_pkt_params);

	lr11xx_radio_set_rf_freq(&lr11xx_hal, lorawan_params.rx1_frequency);

	delay(lorawan_params.rx1_delay - 200);

	if (rx_func != NULL)
	{
		rx_func();
	}
	lr11xx_radio_set_rx(&lr11xx_hal, 500);

	while (1)
	{
		lr11xx_system_get_status(&lr11xx_hal, &stat1, &stat2, &irq_status);
		if (irq_status & LR11XX_SYSTEM_IRQ_RX_DONE)
		{
			break;
		}
		if (irq_status & LR11XX_SYSTEM_IRQ_TIMEOUT)
		{
			break;
		}
	}

	lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

	if (irq_status & LR11XX_SYSTEM_IRQ_RX_DONE)
	{
		receive_downlink = true;
	}

	if (!receive_downlink)
	{
		// RX2 window
		lr11xx_radio_mod_params_lora_t lora_mod_params;
		lora_mod_params.sf = map_spreading_factor(lorawan_params.rx2_spreading_factor);
		lora_mod_params.bw = map_bandwidth(lorawan_params.rx2_bandwidth);
		lora_mod_params.cr = map_coding_rate(lora_params.coding_rate);
		lora_mod_params.ldro = compute_lora_ldro();
		lr11xx_radio_set_lora_mod_params(&lr11xx_hal, &lora_mod_params);

		lr11xx_radio_set_rf_freq(&lr11xx_hal, lorawan_params.rx2_frequency);

		delay(800);

		lr11xx_radio_set_rx(&lr11xx_hal, 500);

		while (1)
		{
			lr11xx_system_get_status(&lr11xx_hal, &stat1, &stat2, &irq_status);
			if (irq_status & LR11XX_SYSTEM_IRQ_RX_DONE)
			{
				break;
			}
			if (irq_status & LR11XX_SYSTEM_IRQ_TIMEOUT)
			{
				break;
			}
		}

		lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

		if (irq_status & LR11XX_SYSTEM_IRQ_RX_DONE)
		{
			receive_downlink = true;
		}
	}

	// parse downlink
	if (!receive_downlink)
	{
		// sleep();
		lorawan_params.rx_length = 0;
		return RFT_STATUS_RX_TIMEOUT;
	}

	lr11xx_radio_pkt_status_lora_t pkt_status;
	lr11xx_radio_get_lora_pkt_status(&lr11xx_hal, &pkt_status);

	lora_params.rssi = pkt_status.rssi_pkt_in_dbm;
	lora_params.snr = pkt_status.snr_pkt_in_db;
	lora_params.signal_rssi = pkt_status.signal_rssi_pkt_in_dbm;

	lr11xx_radio_rx_buffer_status_t rx_buffer;
	lr11xx_radio_get_rx_buffer_status(&lr11xx_hal, &rx_buffer);

	payload_len = rx_buffer.pld_len_in_bytes;
	lr11xx_regmem_read_buffer8(&lr11xx_hal, payload, rx_buffer.buffer_start_pointer, payload_len);

	// sleep();

	return parse_downlink(payload, payload_len);
}

rft_status_t rfthings_lr11xx::send_join_request(void (*tx_func)(), void (*rx_func)())
{
	// buld join request
	unsigned char lorawan_packet[9 + 255 + 4];

	uint8_t packet_len = build_join_request(lorawan_packet);

	// send LoRa
	lr11xx_hal_wakeup(&lr11xx_hal);
	lr11xx_system_set_standby(&lr11xx_hal, LR11XX_SYSTEM_STANDBY_CFG_RC);

	uint32_t irq_status;
	lr11xx_system_stat1_t stat1;
	lr11xx_system_stat2_t stat2;
	lr11xx_system_clear_errors(&lr11xx_hal);

	lr11xx_radio_set_rf_freq(&lr11xx_hal, lora_params.frequency);
	lr11xx_radio_set_tx_params(&lr11xx_hal, lora_params.tx_power, LR11XX_RADIO_RAMP_48_US);

	lr11xx_radio_set_pkt_type(&lr11xx_hal, LR11XX_RADIO_PKT_TYPE_LORA);

	lr11xx_radio_mod_params_lora_t lora_mod_params;
	lora_mod_params.sf = map_spreading_factor(lora_params.spreading_factor);
	lora_mod_params.bw = map_bandwidth(lora_params.bandwidth);
	lora_mod_params.cr = map_coding_rate(lora_params.coding_rate);
	lora_mod_params.ldro = compute_lora_ldro();
	lr11xx_radio_set_lora_mod_params(&lr11xx_hal, &lora_mod_params);

	lr11xx_radio_set_lora_sync_word(&lr11xx_hal, lora_params.syncword);

	lr11xx_radio_pkt_params_lora_t lora_pkt_params;
	lora_pkt_params.preamble_len_in_symb = 32;
	lora_pkt_params.header_type = LR11XX_RADIO_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = packet_len;
	lora_pkt_params.crc = LR11XX_RADIO_LORA_CRC_ON;
	lora_pkt_params.iq = LR11XX_RADIO_LORA_IQ_STANDARD;
	lr11xx_radio_set_lora_pkt_params(&lr11xx_hal, &lora_pkt_params);

	lr11xx_radio_set_lora_public_network(&lr11xx_hal, LR11XX_RADIO_LORA_NETWORK_PUBLIC);

	lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

	lr11xx_regmem_write_buffer8(&lr11xx_hal, lorawan_packet, packet_len);

	lr11xx_radio_set_tx(&lr11xx_hal, 5000);

	while (1)
	{
		lr11xx_system_get_status(&lr11xx_hal, &stat1, &stat2, &irq_status);
		if (irq_status & LR11XX_SYSTEM_IRQ_TX_DONE)
		{
			break;
		}
		if (irq_status & LR11XX_SYSTEM_IRQ_TIMEOUT)
		{
			return RFT_STATUS_TX_TIMEOUT;
		}
	}

	lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

	lorawan_params.framecounter_uplink++;

	// receive downlink
	bool receive_downlink = false;

	// RX1 window
	lora_pkt_params.preamble_len_in_symb = 32;
	lora_pkt_params.header_type = LR11XX_RADIO_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = 255;
	lora_pkt_params.crc = LR11XX_RADIO_LORA_CRC_ON;
	lora_pkt_params.iq = LR11XX_RADIO_LORA_IQ_INVERTED;
	lr11xx_radio_set_lora_pkt_params(&lr11xx_hal, &lora_pkt_params);

	lr11xx_radio_set_rf_freq(&lr11xx_hal, lorawan_params.rx1_frequency);

	delay(lorawan_params.rx1_delay - 200);

	lr11xx_radio_set_rx(&lr11xx_hal, 500);

	while (1)
	{
		lr11xx_system_get_status(&lr11xx_hal, &stat1, &stat2, &irq_status);
		Serial.print("");
		if (irq_status & LR11XX_SYSTEM_IRQ_RX_DONE)
		{
			break;
		}
		if (irq_status & LR11XX_SYSTEM_IRQ_TIMEOUT)
		{
			break;
		}
	}

	lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

	if (irq_status & LR11XX_SYSTEM_IRQ_RX_DONE)
	{
		receive_downlink = true;
	}

	if (!receive_downlink)
	{
		// RX2 window
		lora_mod_params.sf = map_spreading_factor(lorawan_params.rx2_spreading_factor);
		lora_mod_params.bw = map_bandwidth(lorawan_params.rx2_bandwidth);
		lora_mod_params.cr = map_coding_rate(lora_params.coding_rate);
		lora_mod_params.ldro = compute_lora_ldro();
		lr11xx_radio_set_lora_mod_params(&lr11xx_hal, &lora_mod_params);

		lr11xx_radio_set_rf_freq(&lr11xx_hal, lorawan_params.rx2_frequency);

		delay(800);

		lr11xx_radio_set_rx(&lr11xx_hal, 500);

		while (1)
		{
			lr11xx_system_get_status(&lr11xx_hal, &stat1, &stat2, &irq_status);
			if (irq_status & LR11XX_SYSTEM_IRQ_RX_DONE)
			{
				break;
			}
			if (irq_status & LR11XX_SYSTEM_IRQ_TIMEOUT)
			{
				break;
			}
		}

		lr11xx_system_clear_irq_status(&lr11xx_hal, LR11XX_SYSTEM_IRQ_ALL_MASK);

		if (irq_status & LR11XX_SYSTEM_IRQ_RX_DONE)
		{
			receive_downlink = true;
		}
	}

	// parse downlink
	if (!receive_downlink)
	{
		sleep();
		lorawan_params.rx_length = 0;
		return RFT_STATUS_RX_TIMEOUT;
	}

	lr11xx_radio_pkt_status_lora_t pkt_status;
	lr11xx_radio_get_lora_pkt_status(&lr11xx_hal, &pkt_status);

	lora_params.rssi = pkt_status.rssi_pkt_in_dbm;
	lora_params.snr = pkt_status.snr_pkt_in_db;
	lora_params.signal_rssi = pkt_status.signal_rssi_pkt_in_dbm;

	lr11xx_radio_rx_buffer_status_t rx_buffer;
	lr11xx_radio_get_rx_buffer_status(&lr11xx_hal, &rx_buffer);

	packet_len = rx_buffer.pld_len_in_bytes;
	lr11xx_regmem_read_buffer8(&lr11xx_hal, lorawan_packet, rx_buffer.buffer_start_pointer, packet_len);

	sleep();

	return parse_join_accept(lorawan_packet, packet_len);
}

void rfthings_lr11xx::scan_gnss_autonomous(uint8_t nb_sat)
{
	// lr11xx_hal_wakeup(&lr11xx_hal);
	// lr11xx_system_set_standby(&lr11xx_hal, LR11XX_SYSTEM_STANDBY_CFG_RC);
	lr11xx_gnss_constellation_mask_t constellation = 0x01 | 0x02; // GPS & Beidu
	lr11xx_gnss_set_constellations_to_use(&lr11xx_hal, constellation);
	lr11xx_gnss_set_scan_mode(&lr11xx_hal, LR11XX_GNSS_SCAN_MODE_0_SINGLE_SCAN_LEGACY);

	lr11xx_gnss_scan_autonomous(&lr11xx_hal, (lr11xx_gnss_date_t)1646497634, LR11XX_GNSS_OPTION_BEST_EFFORT,
															LR11XX_GNSS_RESULTS_LEGACY_PSEUDO_RANGE_MASK | LR11XX_GNSS_RESULTS_LEGACY_DOPPLER_MASK | LR11XX_GNSS_RESULTS_LEGACY_BIT_CHANGE_MASK, nb_sat);
}

void rfthings_lr11xx::get_gnss_satelites(uint8_t *nb_detected_satellites, lr11xx_gnss_detected_satellite_t *detected_satellite_id_snr_doppler)
{
	lr11xx_gnss_get_nb_detected_satellites(&lr11xx_hal, nb_detected_satellites);
	lr11xx_gnss_get_detected_satellites(&lr11xx_hal, *nb_detected_satellites, detected_satellite_id_snr_doppler);
}

void rfthings_lr11xx::get_nav_message(uint8_t *nav_message, uint16_t *nav_message_len)
{
	lr11xx_gnss_get_result_size(&lr11xx_hal, nav_message_len);
	lr11xx_gnss_read_results(&lr11xx_hal, nav_message, *nav_message_len);
}

rft_status_t rfthings_lr11xx::update_firmware(lr11xx_fw_update_t update, const uint32_t *buffer, uint32_t length)
{
	if (!lr11xx_is_fw_compatible_with_chip(update, lr11xx_bootloader_version.fw))
	{
		return RFT_STATUS_ERROR_WRONG_LR11XX_FIRMWARE_VERSION;
	}

	lr11xx_system_reset(&lr11xx_hal);

	lr11xx_bootloader_erase_flash(&lr11xx_hal);

	lr11xx_bootloader_write_flash_encrypted_full(&lr11xx_hal, 0, buffer, length);

	lr11xx_bootloader_reboot(&lr11xx_hal, false);

	return check_hardware();
}

bool rfthings_lr11xx::lr11xx_is_fw_compatible_with_chip(lr11xx_fw_update_t update, uint16_t bootloader_version)
{
	if (update == LR1110_FIRMWARE_UPDATE_TO_TRX && bootloader_version != 0x6500)
	{
		return false;
	}
	if (update == LR1120_FIRMWARE_UPDATE_TO_TRX && bootloader_version != 0x2000)
	{
		return false;
	}

	return true;
}

void rfthings_lr11xx::config_continous_wave(void)
{
#define RF_FREQ_IN_HZ 868000000
#define TX_OUTPUT_POWER_DBM 22
#define PA_RAMP_TIME LR11XX_RADIO_RAMP_48_US
#define FALLBACK_MODE LR11XX_RADIO_FALLBACK_STDBY_RC
#define ENABLE_RX_BOOST_MODE false
#define LORA_SYNCWORD 0x12 // 0x12 Private Network, 0x34 Public Network

#define LORA_SPREADING_FACTOR LR11XX_RADIO_LORA_SF7
#define LORA_BANDWIDTH LR11XX_RADIO_LORA_BW_125
#define LORA_CODING_RATE LR11XX_RADIO_LORA_CR_4_5

#define PAYLOAD_LENGTH 7
#define LORA_PREAMBLE_LENGTH 8
#define LORA_PKT_LEN_MODE LR11XX_RADIO_LORA_PKT_EXPLICIT
#define LORA_IQ LR11XX_RADIO_LORA_IQ_INVERTED
#define LORA_CRC LR11XX_RADIO_LORA_CRC_ON

	lr11xx_radio_set_pkt_type(&lr11xx_hal, LR11XX_RADIO_PKT_TYPE_LORA);
	lr11xx_radio_set_rf_freq(&lr11xx_hal, RF_FREQ_IN_HZ);
	lr11xx_radio_set_rssi_calibration(&lr11xx_hal, &rssi_calibration_table_from_600mhz_to_2ghz);

	lr11xx_radio_pa_cfg_t cw_pa_config;
	cw_pa_config.pa_sel = LR11XX_RADIO_PA_SEL_HF;
	cw_pa_config.pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT;
	cw_pa_config.pa_duty_cycle = 0x04;
	cw_pa_config.pa_hp_sel = 0x07;
	lr11xx_radio_set_pa_cfg(&lr11xx_hal, &cw_pa_config);

	lr11xx_radio_set_tx_params(&lr11xx_hal, TX_OUTPUT_POWER_DBM, PA_RAMP_TIME);
	lr11xx_radio_set_rx_tx_fallback_mode(&lr11xx_hal, FALLBACK_MODE);
	lr11xx_radio_cfg_rx_boosted(&lr11xx_hal, ENABLE_RX_BOOST_MODE);

	const lr11xx_radio_mod_params_lora_t lora_mod_params = {
			.sf = LORA_SPREADING_FACTOR,
			.bw = LORA_BANDWIDTH,
			.cr = LORA_CODING_RATE,
			.ldro = compute_lora_ldro(RFT_LORA_SPREADING_FACTOR_7, RFT_LORA_BANDWIDTH_125KHZ),
	};

	const lr11xx_radio_pkt_params_lora_t lora_pkt_params = {
			.preamble_len_in_symb = LORA_PREAMBLE_LENGTH,
			.header_type = LORA_PKT_LEN_MODE,
			.pld_len_in_bytes = PAYLOAD_LENGTH,
			.crc = LORA_CRC,
			.iq = LORA_IQ,
	};

	lr11xx_radio_set_lora_mod_params(&lr11xx_hal, &lora_mod_params);
	lr11xx_radio_set_lora_pkt_params(&lr11xx_hal, &lora_pkt_params);
	lr11xx_radio_set_lora_sync_word(&lr11xx_hal, LORA_SYNCWORD);
}

void rfthings_lr11xx::start_continuous_wave(void)
{
	lr11xx_hal_wakeup(&lr11xx_hal);
	lr11xx_system_set_standby(&lr11xx_hal, LR11XX_SYSTEM_STANDBY_CFG_RC);

	lr11xx_radio_set_tx_cw(&lr11xx_hal);
}

void rfthings_lr11xx::stop_continuous_wave(void)
{
	lr11xx_system_set_standby(&lr11xx_hal, LR11XX_SYSTEM_STANDBY_CFG_RC);
	sleep();
}

rft_status_t rfthings_lr11xx::sweep_continuous_wave(uint32_t start_freq, uint32_t stop_freq, uint32_t step, uint16_t duration)
{
	lr11xx_hal_wakeup(&lr11xx_hal);
	lr11xx_system_set_standby(&lr11xx_hal, LR11XX_SYSTEM_STANDBY_CFG_RC);

	if (start_freq != 0 && stop_freq != 0 && stop_freq > start_freq && step > 0 && duration > 0)
	{
		uint32_t current_freq = start_freq;

		while (current_freq < stop_freq)
		{
			lr11xx_radio_set_rf_freq(&lr11xx_hal, current_freq);
			delay(1);

			lr11xx_radio_set_tx_cw(&lr11xx_hal);
			delay(duration);

			current_freq += step;
			lr11xx_system_set_standby(&lr11xx_hal, LR11XX_SYSTEM_STANDBY_CFG_RC);
		}
		stop_continuous_wave();

		return RFT_STATUS_OK;
	}

	return RFT_STATUS_ERROR_INVALID_PARAM;
}

rft_status_t rfthings_lr11xx::check_hardware(void)
{
	lr11xx_system_get_version(&lr11xx_hal, &lr11xx_system_version);
	lr11xx_bootloader_get_version(&lr11xx_hal, &lr11xx_bootloader_version);

	lr11xx_bootloader_read_pin(&lr11xx_hal, pin);
	lr11xx_bootloader_read_chip_eui(&lr11xx_hal, chip_eui);
	lr11xx_bootloader_read_join_eui(&lr11xx_hal, join_eui);

	if (lr11xx_system_version.type != 0x01 && lr11xx_system_version.type != 0x02)
	{
		return RFT_STATUS_ERROR_HARDWARE;
	}
	return RFT_STATUS_OK;
}

uint8_t rfthings_lr11xx::get_system_hardware(void)
{
	return lr11xx_system_version.hw;
}

uint8_t rfthings_lr11xx::get_system_type(void)
{
	return lr11xx_system_version.type;
}

uint16_t rfthings_lr11xx::get_system_firmware(void)
{
	return lr11xx_system_version.fw;
}

uint8_t rfthings_lr11xx::get_bootloader_hardware(void)
{
	return lr11xx_bootloader_version.hw;
}

uint8_t rfthings_lr11xx::get_bootloader_type(void)
{
	return lr11xx_bootloader_version.type;
}

uint16_t rfthings_lr11xx::get_bootloader_firmware(void)
{
	return lr11xx_bootloader_version.fw;
}

uint8_t rfthings_lr11xx::get_gnss_firmware(void)
{
	return lr11xx_gnss_version.gnss_firmware;
}

uint8_t rfthings_lr11xx::get_gnss_almanac(void)
{
	return lr11xx_gnss_version.gnss_almanac;
}

uint8_t *rfthings_lr11xx::get_pin(void)
{
	return pin;
}

uint8_t *rfthings_lr11xx::get_chip_eui(void)
{
	return chip_eui;
}

uint8_t *rfthings_lr11xx::get_join_eui(void)
{
	return join_eui;
}

lr11xx_radio_lora_sf_t rfthings_lr11xx::map_spreading_factor(rft_lora_spreading_factor_t spreading_factor)
{
	switch (spreading_factor)
	{
	case RFT_LORA_SPREADING_FACTOR_5:
		return LR11XX_RADIO_LORA_SF5;
		break;
	case RFT_LORA_SPREADING_FACTOR_6:
		return LR11XX_RADIO_LORA_SF6;
		break;
	case RFT_LORA_SPREADING_FACTOR_7:
		return LR11XX_RADIO_LORA_SF7;
		break;
	case RFT_LORA_SPREADING_FACTOR_8:
		return LR11XX_RADIO_LORA_SF8;
		break;
	case RFT_LORA_SPREADING_FACTOR_9:
		return LR11XX_RADIO_LORA_SF9;
		break;
	case RFT_LORA_SPREADING_FACTOR_10:
		return LR11XX_RADIO_LORA_SF10;
		break;
	case RFT_LORA_SPREADING_FACTOR_11:
		return LR11XX_RADIO_LORA_SF11;
		break;
	case RFT_LORA_SPREADING_FACTOR_12:
		return LR11XX_RADIO_LORA_SF12;
		break;
	default:
		return LR11XX_RADIO_LORA_SF7;
		break;
	}
}

lr11xx_radio_lora_bw_t rfthings_lr11xx::map_bandwidth(rft_lora_bandwidth_t bandwidth)
{
	switch (bandwidth)
	{
	case RFT_LORA_BANDWIDTH_10KHZ:
		return LR11XX_RADIO_LORA_BW_10;
		break;
	case RFT_LORA_BANDWIDTH_15KHZ:
		return LR11XX_RADIO_LORA_BW_15;
		break;
	case RFT_LORA_BANDWIDTH_20KHZ:
		return LR11XX_RADIO_LORA_BW_20;
		break;
	case RFT_LORA_BANDWIDTH_31KHZ:
		return LR11XX_RADIO_LORA_BW_31;
		break;
	case RFT_LORA_BANDWIDTH_41KHZ:
		return LR11XX_RADIO_LORA_BW_41;
		break;
	case RFT_LORA_BANDWIDTH_62KHZ:
		return LR11XX_RADIO_LORA_BW_62;
		break;
	case RFT_LORA_BANDWIDTH_125KHZ:
		return LR11XX_RADIO_LORA_BW_125;
		break;
	case RFT_LORA_BANDWIDTH_250KHZ:
		return LR11XX_RADIO_LORA_BW_250;
		break;
	case RFT_LORA_BANDWIDTH_500KHZ:
		return LR11XX_RADIO_LORA_BW_500;
		break;
	case RFT_LORA_BANDWIDTH_200KHZ:
		return LR11XX_RADIO_LORA_BW_200;
		break;
	case RFT_LORA_BANDWIDTH_400KHZ:
		return LR11XX_RADIO_LORA_BW_400;
		break;
	case RFT_LORA_BANDWIDTH_800KHZ:
		return LR11XX_RADIO_LORA_BW_800;
		break;
	default:
		return LR11XX_RADIO_LORA_BW_125;
		break;
	}
}

lr11xx_radio_lora_cr_t rfthings_lr11xx::map_coding_rate(rft_lora_coding_rate_t coding_rate)
{
	switch (coding_rate)
	{
	case RFT_LORA_CODING_RATE_4_5:
		return LR11XX_RADIO_LORA_CR_4_5;
		break;
	case RFT_LORA_CODING_RATE_4_6:
		return LR11XX_RADIO_LORA_CR_4_6;
		break;
	case RFT_LORA_CODING_RATE_4_7:
		return LR11XX_RADIO_LORA_CR_4_7;
		break;
	case RFT_LORA_CODING_RATE_4_8:
		return LR11XX_RADIO_LORA_CR_4_8;
		break;
	default:
		return LR11XX_RADIO_LORA_CR_4_5;
		break;
	}
}

/*!
 * @brief A function to get the value for low data rate optimization setting
 *
 * @param [in] sf  LoRa Spreading Factor
 * @param [in] bw  LoRa Bandwidth
 */
uint8_t rfthings_lr11xx::compute_lora_ldro(void)
{
	return (compute_lora_ldro(lora_params.spreading_factor, lora_params.bandwidth));
}
uint8_t rfthings_lr11xx::compute_lora_ldro(rft_lora_spreading_factor_t _spreading_factor, rft_lora_bandwidth_t _bandwidth)
{
	switch (_bandwidth)
	{
	case RFT_LORA_BANDWIDTH_500KHZ:
		return 0;

	case RFT_LORA_BANDWIDTH_250KHZ:
		if (_spreading_factor == RFT_LORA_SPREADING_FACTOR_12)
		{
			return 1;
		}
		else
		{
			return 0;
		}

	case RFT_LORA_BANDWIDTH_800KHZ:
	case RFT_LORA_BANDWIDTH_400KHZ:
	case RFT_LORA_BANDWIDTH_200KHZ:
	case RFT_LORA_BANDWIDTH_125KHZ:
		if ((_spreading_factor == RFT_LORA_SPREADING_FACTOR_12) || (_spreading_factor == RFT_LORA_SPREADING_FACTOR_11))
		{
			return 1;
		}
		else
		{
			return 0;
		}

	case RFT_LORA_BANDWIDTH_62KHZ:
		if ((_spreading_factor == RFT_LORA_SPREADING_FACTOR_12) || (_spreading_factor == RFT_LORA_SPREADING_FACTOR_11) ||
				(_spreading_factor == RFT_LORA_SPREADING_FACTOR_10))
		{
			return 1;
		}
		else
		{
			return 0;
		}

	case RFT_LORA_BANDWIDTH_41KHZ:
		if ((_spreading_factor == RFT_LORA_SPREADING_FACTOR_12) || (_spreading_factor == RFT_LORA_SPREADING_FACTOR_11) ||
				(_spreading_factor == RFT_LORA_SPREADING_FACTOR_10) || (_spreading_factor == RFT_LORA_SPREADING_FACTOR_9))
		{
			return 1;
		}
		else
		{
			return 0;
		}

	case RFT_LORA_BANDWIDTH_31KHZ:
	case RFT_LORA_BANDWIDTH_20KHZ:
	case RFT_LORA_BANDWIDTH_15KHZ:
	case RFT_LORA_BANDWIDTH_10KHZ:
		// case LR11XX_RADIO_LORA_BW_7:
		return 1;

	default:
		return 0;
	}
}

lr11xx_radio_rssi_calibration_table_t *rfthings_lr11xx::get_rssi_calibration_table(void)
{
	if (lora_params.frequency < 600000000)
	{
		return &rssi_calibration_table_below_600mhz;
	}
	else if ((600000000 <= lora_params.frequency) && (lora_params.frequency <= 2000000000))
	{
		return &rssi_calibration_table_from_600mhz_to_2ghz;
	}
	else
	{
		return &rssi_calibration_table_above_2ghz;
	}
}

lr11xx_radio_rssi_calibration_table_t rfthings_lr11xx::rssi_calibration_table_below_600mhz = {
		.gain_tune = {.g4 = 12,
									.g5 = 12,
									.g6 = 14,
									.g7 = 0,
									.g8 = 1,
									.g9 = 3,
									.g10 = 4,
									.g11 = 4,
									.g12 = 3,
									.g13 = 6,
									.g13hp1 = 6,
									.g13hp2 = 6,
									.g13hp3 = 6,
									.g13hp4 = 6,
									.g13hp5 = 6,
									.g13hp6 = 6,
									.g13hp7 = 6},
		.gain_offset = 0,
};

lr11xx_radio_rssi_calibration_table_t rfthings_lr11xx::rssi_calibration_table_from_600mhz_to_2ghz = {
		.gain_tune = {.g4 = 2,
									.g5 = 2,
									.g6 = 2,
									.g7 = 3,
									.g8 = 3,
									.g9 = 4,
									.g10 = 5,
									.g11 = 4,
									.g12 = 4,
									.g13 = 6,
									.g13hp1 = 5,
									.g13hp2 = 5,
									.g13hp3 = 6,
									.g13hp4 = 6,
									.g13hp5 = 6,
									.g13hp6 = 7,
									.g13hp7 = 6},
		.gain_offset = 0,
};

lr11xx_radio_rssi_calibration_table_t rfthings_lr11xx::rssi_calibration_table_above_2ghz = {
		.gain_tune = {.g4 = 6,
									.g5 = 7,
									.g6 = 6,
									.g7 = 4,
									.g8 = 3,
									.g9 = 4,
									.g10 = 14,
									.g11 = 12,
									.g12 = 14,
									.g13 = 12,
									.g13hp1 = 12,
									.g13hp2 = 12,
									.g13hp3 = 12,
									.g13hp4 = 8,
									.g13hp5 = 8,
									.g13hp6 = 9,
									.g13hp7 = 9},
		.gain_offset = 2030,
};