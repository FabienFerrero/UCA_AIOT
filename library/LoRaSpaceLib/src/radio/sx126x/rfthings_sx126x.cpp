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

#include "rfthings_sx126X.h"
#include "./sx126x_driver/lr_fhss_mac.h"
#include "./sx126x_driver/sx126x_lr_fhss.h"

#define SX126X_XTAL_FREQ (32000000UL)
#define SX126X_FREQ_STEP (0.95367431640625)

volatile bool detect_preamble;

rfthings_sx126x::rfthings_sx126x(byte nss_pin, byte rst_pin, byte busy_pin, byte dio1_pin, byte antenna_switch_pin)
{
	sx126x_hal.nss = nss_pin;
	sx126x_hal.reset = rst_pin;
	sx126x_hal.busy = busy_pin;
	sx126x_hal.irq = dio1_pin;
	sx126x_hal.antenna_switch = antenna_switch_pin;
}

rfthings_sx126x::~rfthings_sx126x(void)
{
}

void rfthings_sx126x::sleep(void)
{
	sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START);
}

void rfthings_sx126x::wake_up(void)
{
	sx126x_hal_wakeup(&sx126x_hal);
}

rft_status_t rfthings_sx126x::init(rft_region_t region)
{
	if (create_params_by_region(region) != RFT_STATUS_OK)
	{
		return RFT_STATUS_ERROR_INVALID_REGION;
	}

	pinMode(sx126x_hal.reset, OUTPUT);
	pinMode(sx126x_hal.busy, INPUT);
	pinMode(sx126x_hal.irq, INPUT_PULLUP);
	pinMode(sx126x_hal.nss, OUTPUT);
	pinMode(sx126x_hal.antenna_switch, OUTPUT);

	SPI.begin();
	digitalWrite(sx126x_hal.nss, HIGH);

	digitalWrite(sx126x_hal.antenna_switch, LOW);

	sx126x_hal_reset(&sx126x_hal);

	if (check_hardware() != RFT_STATUS_OK)
	{
		return RFT_STATUS_ERROR_HARDWARE;
	}

	sx126x_set_reg_mode(&sx126x_hal, SX126X_REG_MODE_DCDC);

	sx126x_cfg_tx_clamp(&sx126x_hal);

	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);

	sx126x_set_dio2_as_rf_sw_ctrl(&sx126x_hal, true);

	sx126x_set_dio3_as_tcxo_ctrl(&sx126x_hal, SX126X_TCXO_CTRL_1_7V, 320);

	sx126x_cal(&sx126x_hal, 0x7f);

	calibrate_image(lora_params.frequency);

	sx126x_set_lora_sync_word(&sx126x_hal, lora_params.syncword);

	return RFT_STATUS_OK;
}

rft_status_t rfthings_sx126x::check_hardware(void)
{
	uint16_t syncword;
	uint8_t buffer[2] = {0x00, 0x00};

	sx126x_read_register(&sx126x_hal, SX126X_REG_LR_SYNCWORD, buffer, 2);

	syncword = ((uint16_t)buffer[0] << 8) | buffer[1];

	if (syncword != 0x1424)
	{
		return RFT_STATUS_ERROR_HARDWARE;
	}

	return RFT_STATUS_OK;
}

rft_status_t rfthings_sx126x::send_lora(byte *payload, uint32_t payload_len, uint32_t timeout, void (*tx_func)())
{
	sx126x_wakeup(&sx126x_hal);
	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);

	sx126x_irq_mask_t irq_status;
	uint16_t symbol_len;

	symbol_len = 8;
	if (lora_params.send_to_relay)
	{
		symbol_len = 8 + (sx126x_get_lora_bw_in_hz(map_bandwidth(lora_params.bandwidth)) / (1 << map_spreading_factor(lora_params.spreading_factor)));
	}

	sx126x_set_pkt_type(&sx126x_hal, SX126X_PKT_TYPE_LORA);

	sx126x_mod_params_lora_t lora_mod_params;
	lora_mod_params.sf = map_spreading_factor(lora_params.spreading_factor);
	lora_mod_params.bw = map_bandwidth(lora_params.bandwidth);
	lora_mod_params.cr = map_coding_rate(lora_params.coding_rate);
	// lora_mod_params.ldro = 0x01;
	lora_mod_params.ldro = compute_lora_ldro();
	sx126x_set_lora_mod_params(&sx126x_hal, &lora_mod_params);

	sx126x_pkt_params_lora_t lora_pkt_params;
	lora_pkt_params.preamble_len_in_symb = symbol_len;
	lora_pkt_params.header_type = SX126X_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = payload_len;
	lora_pkt_params.crc_is_on = true;
	lora_pkt_params.invert_iq_is_on = false;
	sx126x_set_lora_pkt_params(&sx126x_hal, &lora_pkt_params);

	sx126x_pa_cfg_params_t pa_config;
	pa_config.pa_duty_cycle = 0x04;
	pa_config.hp_max = 0x07;
	pa_config.device_sel = 0x00;
	pa_config.pa_lut = 0x01;
	sx126x_set_pa_cfg(&sx126x_hal, &pa_config);

	sx126x_set_ocp_value(&sx126x_hal, 0x38);

	sx126x_set_tx_params(&sx126x_hal, lora_params.tx_power, SX126X_RAMP_10_US);
	sx126x_set_rf_freq(&sx126x_hal, lora_params.frequency);

	sx126x_set_dio_irq_params(&sx126x_hal, 0x01, 0x01, 0x00, 0x00);
	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

	sx126x_set_buffer_base_address(&sx126x_hal, 0x00, 0x00);
	sx126x_write_buffer(&sx126x_hal, 0x00, payload, payload_len);

	if (tx_func != NULL)
	{
		tx_func();
	}
	sx126x_set_tx(&sx126x_hal, timeout);

	while (1)
	{
		sx126x_get_irq_status(&sx126x_hal, &irq_status);
		if (irq_status & SX126X_IRQ_TX_DONE)
		{
			break;
		}
		if (irq_status & SX126X_IRQ_TIMEOUT)
		{
			return RFT_STATUS_TX_TIMEOUT;
		}
	}

	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

	// sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START);

	return RFT_STATUS_TX_DONE;
}

rft_status_t rfthings_sx126x::receive_lora(byte *payload, uint32_t payload_len, uint32_t timeout, void (*rx_func)())
{
	sx126x_wakeup(&sx126x_hal);
	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);

	sx126x_irq_mask_t irq_status;

	sx126x_set_pkt_type(&sx126x_hal, SX126X_PKT_TYPE_LORA);

	digitalWrite(sx126x_hal.antenna_switch, HIGH);

	sx126x_mod_params_lora_t lora_mod_params;
	lora_mod_params.sf = map_spreading_factor(lora_params.spreading_factor);
	lora_mod_params.bw = map_bandwidth(lora_params.bandwidth);
	lora_mod_params.cr = map_coding_rate(lora_params.coding_rate);
	lora_mod_params.ldro = compute_lora_ldro();
	sx126x_set_lora_mod_params(&sx126x_hal, &lora_mod_params);

	sx126x_pkt_params_lora_t lora_pkt_params;
	lora_pkt_params.preamble_len_in_symb = 8;
	lora_pkt_params.header_type = SX126X_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = 255;
	lora_pkt_params.crc_is_on = true;
	lora_pkt_params.invert_iq_is_on = false;
	sx126x_set_lora_pkt_params(&sx126x_hal, &lora_pkt_params);

	sx126x_cfg_rx_boosted(&sx126x_hal, true);

	sx126x_set_rf_freq(&sx126x_hal, lora_params.frequency);

	sx126x_set_dio_irq_params(&sx126x_hal, 0x0fff, 0x0fff, 0x00, 0x00);

	sx126x_set_buffer_base_address(&sx126x_hal, 0x00, 0x00);

	if (rx_func != NULL)
	{
		rx_func();
	}
	sx126x_set_rx(&sx126x_hal, timeout);

	while (1)
	{
		sx126x_get_irq_status(&sx126x_hal, &irq_status);
		if (irq_status & SX126X_IRQ_RX_DONE)
		{
			break;
		}
		if (irq_status & SX126X_IRQ_TIMEOUT)
		{
			break;
		}
	}

	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

	if (irq_status & SX126X_IRQ_TIMEOUT)
	{
		return RFT_STATUS_RX_TIMEOUT;
	}

	sx126x_pkt_status_lora_t pkt_status;
	sx126x_get_lora_pkt_status(&sx126x_hal, &pkt_status);

	lora_params.rssi = pkt_status.rssi_pkt_in_dbm;
	lora_params.snr = pkt_status.snr_pkt_in_db;
	lora_params.signal_rssi = pkt_status.signal_rssi_pkt_in_dbm;

	sx126x_rx_buffer_status_t rx_buffer_status;
	sx126x_get_rx_buffer_status(&sx126x_hal, &rx_buffer_status);

	payload_len = rx_buffer_status.pld_len_in_bytes;
	sx126x_read_buffer(&sx126x_hal, rx_buffer_status.buffer_start_pointer, payload, payload_len);

	lorawan_params.rx_length = payload_len;

	return RFT_STATUS_OK;
}

void rfthings_sx126x::set_lora_pkt_param(sx126x_pkt_params_lora_t param)
{
	sx126x_set_lora_pkt_params(&sx126x_hal, &param);
}

rft_status_t rfthings_sx126x::relay(byte *payload, uint32_t &payload_len, void (*rx_func)(), void (*sleep_func)())
{
	return (this->relay(&(this->lora_params), payload, payload_len, rx_func, sleep_func));
}

rft_status_t rfthings_sx126x::relay(rft_lora_params_t *relay_lora_params, byte *payload, uint32_t &payload_len, void (*rx_func)(), void (*sleep_func)())
{
	/**
	 * TODO: Parameters validation!
	 * relay_lora_params: Nom NULL Pointer, also need to validate following values:
	 *    +	relay_sleep_interval_us
	 *    + relay_rx_symbol
	 *    + relay_max_rx_packet_length
	 *
	 * payload: Non NULL pointer
	 */

	rft_status_t return_value = RFT_STATUS_OK;

	sx126x_wakeup(&sx126x_hal);
	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);

	// Preable length for Relay application
	uint16_t preamble_length = 8 + (sx126x_get_lora_bw_in_hz(map_bandwidth(relay_lora_params->bandwidth)) / (1 << map_spreading_factor(relay_lora_params->spreading_factor)));

	sx126x_set_pkt_type(&sx126x_hal, SX126X_PKT_TYPE_LORA);

	digitalWrite(sx126x_hal.antenna_switch, HIGH);

	sx126x_mod_params_lora_t lora_mod_params;
	lora_mod_params.sf = map_spreading_factor(relay_lora_params->spreading_factor);
	lora_mod_params.bw = map_bandwidth(relay_lora_params->bandwidth);
	lora_mod_params.cr = map_coding_rate(relay_lora_params->coding_rate);
	lora_mod_params.ldro = compute_lora_ldro(*relay_lora_params);
	sx126x_set_lora_mod_params(&sx126x_hal, &lora_mod_params);

	sx126x_pkt_params_lora_t lora_pkt_params;
	lora_pkt_params.preamble_len_in_symb = preamble_length; // Calculated
	lora_pkt_params.header_type = SX126X_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = relay_lora_params->relay_max_rx_packet_length;
	lora_pkt_params.crc_is_on = true;
	lora_pkt_params.invert_iq_is_on = false;
	sx126x_set_lora_pkt_params(&sx126x_hal, &lora_pkt_params);

	sx126x_set_lora_sync_word(&sx126x_hal, relay_lora_params->syncword);

	sx126x_cfg_rx_boosted(&sx126x_hal, true);

	sx126x_set_rf_freq(&sx126x_hal, relay_lora_params->frequency);

	sx126x_set_dio_irq_params(&sx126x_hal, 0x0fff, 0x0fff, 0x00, 0x00);

	sx126x_set_buffer_base_address(&sx126x_hal, 0x00, 0x00);

	sx126x_set_reg_mode(&sx126x_hal, SX126X_REG_MODE_DCDC);

	sx126x_set_lora_symb_nb_timeout(&sx126x_hal, 0);

	if (rx_func != NULL)
	{
		rx_func();
	}

	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);
	detect_preamble = false;
	attachInterrupt(digitalPinToInterrupt(sx126x_hal.irq), (voidFuncPtr)(rfthings_sx126x::irq_relay), RISING);

	uint32_t rxTime = (1e6 * relay_lora_params->relay_rx_symbol * (1 << map_spreading_factor(relay_lora_params->spreading_factor)) / sx126x_get_lora_bw_in_hz(map_bandwidth(relay_lora_params->bandwidth))) / T_STEP;
	uint32_t sleepTime = (relay_lora_params->relay_sleep_interval_us) / T_STEP - rxTime;
	sx126x_set_rx_duty_cycle_with_timings_in_rtc_step(&sx126x_hal, rxTime, sleepTime);

	if (sleep_func != NULL)
	{
		sleep_func();
	}
	else
	{
		while (!detect_preamble)
		{
		};
		detect_preamble = false;
	}
	detachInterrupt(digitalPinToInterrupt(sx126x_hal.irq));

	sx126x_irq_mask_t irq_status;
	sx126x_get_irq_status(&sx126x_hal, &irq_status);
	if (irq_status & SX126X_IRQ_PREAMBLE_DETECTED)
	{
		sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);

		sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

		sx126x_set_rx(&sx126x_hal, 2000);

		while (1)
		{
			sx126x_get_irq_status(&sx126x_hal, &irq_status);
			if (irq_status & SX126X_IRQ_RX_DONE)
			{
				break;
			}
			if (irq_status & SX126X_IRQ_TIMEOUT)
			{
				break;
			}
			delay(10);
		}

		if (irq_status & SX126X_IRQ_TIMEOUT)
		{
			// Timeout
			payload_len = 0;
			return_value = RFT_STATUS_RX_TIMEOUT;
		}
		else
		{
			// Rx Done
			sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);

			sx126x_pkt_status_lora_t pkt_status;
			sx126x_get_lora_pkt_status(&sx126x_hal, &pkt_status);

			relay_lora_params->rssi = pkt_status.rssi_pkt_in_dbm;
			relay_lora_params->snr = pkt_status.snr_pkt_in_db;
			relay_lora_params->signal_rssi = pkt_status.signal_rssi_pkt_in_dbm;

			sx126x_rx_buffer_status_t rx_buffer_status;
			sx126x_get_rx_buffer_status(&sx126x_hal, &rx_buffer_status);

			payload_len = rx_buffer_status.pld_len_in_bytes;
			sx126x_read_buffer(&sx126x_hal, rx_buffer_status.buffer_start_pointer, payload, payload_len);

			return_value = RFT_STATUS_OK;
		}
	}
	else
	{
		payload_len = 0;
		return_value = RFT_STATUS_PREAMBLE_DETECT_FAIL;
	}

	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);
	digitalWrite(sx126x_hal.antenna_switch, 0);

	sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START);

	return return_value;
}

rft_status_t rfthings_sx126x::send_lrfhss(byte *payload, uint32_t payload_len)
{
	return (this->send_lrfhss(this->lrfhss_params, payload, payload_len, NULL));
}

rft_status_t rfthings_sx126x::send_lrfhss(byte *payload, uint32_t payload_len, void (*tx_func)())
{
	return (this->send_lrfhss(this->lrfhss_params, payload, payload_len, tx_func));
}

rft_status_t rfthings_sx126x::send_lrfhss(rft_lrfhss_params_t params, byte *payload, uint32_t payload_len)
{
	return (this->send_lrfhss(params, payload, payload_len));
}

rft_status_t rfthings_sx126x::send_lrfhss(rft_lrfhss_params_t params, byte *payload, uint32_t payload_len, void (*tx_func)())
{
#define LS_MAXPAYLOADOUTSIZE 608 // 3*(8*(maxpayload+2)+6)/8, maxpayload = 200 (?)
#define LS_NBFREQHOPMAX 64
#define LS_SYNCSIZE 114
#define LS_BLOCKSIZEV3 50

	byte LSBuffer[LS_MAXPAYLOADOUTSIZE];
	memset(LSBuffer, 0, sizeof LSBuffer);

	int8_t power;

	uint8_t nbBlock;
	uint16_t nbBits;
	uint16_t state;
	int16_t hop;
	int freqHopping[LS_NBFREQHOPMAX];

	const uint8_t pulse_shape_compensation = 1;

	/*!
	 * LoRaWAN LR-FHSS sync word definition
	 */
	const uint8_t lr_fhss_sync_word[4] = {0x2C, 0x0F, 0x79, 0x95};

	lr_fhss_v1_params_t v1_params;
	lr_fhss_digest_t digest;
	lr_fhss_hop_params_t hop_params;

	v1_params.sync_word = lr_fhss_sync_word;
	v1_params.modulation_type = LR_FHSS_V1_MODULATION_TYPE_GMSK_488;

	v1_params.cr = (lr_fhss_v1_cr_e)params.codingRate;
	v1_params.grid = (lr_fhss_v1_grid_e)params.grid;
	v1_params.bw = (lr_fhss_v1_bw_e)params.bandwidth;
	v1_params.enable_hopping = params.hopping;
	v1_params.header_count = params.nbSync; /**< Number of header blocks */

	uint16_t hop_sequence_count = lr_fhss_get_hop_sequence_count(&v1_params);
	uint16_t hoppingPatternSeq = this->get_random_number() % hop_sequence_count;
	uint16_t payloadOutSize = lr_fhss_build_frame(&v1_params, hoppingPatternSeq, payload, payload_len, LSBuffer);

	if (payloadOutSize > 255)
		return RFT_STATUS_ERROR_PAYLOAD_TOO_LONG;

	lr_fhss_process_parameters(&v1_params, payload_len, &digest);

	nbBlock = digest.nb_hops;
	nbBits = digest.nb_bits + pulse_shape_compensation; // offset for pulse_shape_compensation (?)

	lr_fhss_get_hop_params(&v1_params, &hop_params, &state, hoppingPatternSeq);

	for (uint32_t i = 0; i < (4 - params.nbSync); i++)
	{
		hop = lr_fhss_get_next_freq_in_grid(&state, &hop_params, &v1_params);
	}

	for (uint32_t i = 0; i < nbBlock; i++)
	{
		hop = lr_fhss_get_next_freq_in_grid(&state, &hop_params, &v1_params);
		freqHopping[i] = hop;
	}

	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);
	sx126x_set_rf_freq(&sx126x_hal, params.frequency);
	while (digitalRead(sx126x_hal.busy))
	{
	}

	// Use DC-DC regulator mode
	sx126x_set_reg_mode(&sx126x_hal, SX126X_REG_MODE_DCDC);

	uint8_t data[1];
	data[0] = 1;
	sx126x_write_register(&sx126x_hal, SX126X_LR_FHSS_REG_CTRL, data, 1);
	data[0] = payloadOutSize;
	sx126x_write_register(&sx126x_hal, SX126X_LR_FHSS_REG_PACKET_LEN, data, 1);
	data[0] = nbBlock;
	sx126x_write_register(&sx126x_hal, SX126X_LR_FHSS_REG_NUM_HOPS, data, 1);

	uint32_t grid_pll_steps = params.grid ? 4096 : 26624;
	uint32_t refFreq_pll = (double)params.frequency / (double)SX126X_FREQ_STEP;

	uint16_t currenthop;
	uint16_t blockSize = LS_BLOCKSIZEV3;

	if (nbBlock > 16)
	{
		currenthop = 16;
	}
	else
	{
		currenthop = nbBlock;
	}

	for (uint8_t l = 0; l < currenthop; l++)
	{

		uint32_t freq = refFreq_pll - freqHopping[l] * grid_pll_steps;

		// offset freq
		if (((params.nbSync - l) % 2) == 0 && l < params.nbSync && params.hopping == 1)
		{
			// freq = freq + ( uint32_t )( ( double ) OFFSET_SYNCWORD / ( double )FREQ_STEP );
			freq = freq + 256;
		}

		uint16_t blockSize = l < params.nbSync ? LS_SYNCSIZE : LS_BLOCKSIZEV3;

		if (!params.hopping)
		{
			blockSize = nbBits;
		}

		// pulse_shape_compensation
		if (l == 0)
			blockSize = blockSize + pulse_shape_compensation;

		if (blockSize > nbBits)
			blockSize = nbBits;

		uint8_t data[] = {(uint8_t)((blockSize >> 8) & 0xff), (uint8_t)(blockSize & 0xff),
											(uint8_t)((freq >> 24) & 0xff), (uint8_t)((freq >> 16) & 0xff), (uint8_t)((freq >> 8) & 0xff), (uint8_t)(freq & 0xff)};
		sx126x_write_register_bulk(&sx126x_hal, SX126X_LR_FHSS_REG_NUM_SYMBOLS_0 + l * 6, data, sizeof data);

		nbBits -= blockSize;
	}

	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);
	sx126x_set_pkt_type(&sx126x_hal, SX126X_PKT_TYPE_LR_FHSS);

	sx126x_lr_fhss_params_t lrfhss_param;
	sx126x_lr_fhss_init(&sx126x_hal, &lrfhss_param);
	
	// setTxPower(params->power);
	sx126x_pa_cfg_params_t pa_config;
	pa_config.pa_duty_cycle = 0x04;
	pa_config.hp_max = 0x07;
	pa_config.device_sel = 0x00;
	pa_config.pa_lut = 0x01;
	sx126x_set_pa_cfg(&sx126x_hal, &pa_config);
	params.power = CAP(params.power, -3, 22);
	sx126x_set_ocp_value(&sx126x_hal, 0x38);
	sx126x_set_tx_params(&sx126x_hal, params.power, SX126X_RAMP_10_US);

	uint16_t irq_mask = (SX126X_IRQ_TX_DONE | SX126X_IRQ_LR_FHSS_HOP);
	sx126x_set_dio_irq_params(&sx126x_hal, irq_mask, irq_mask, 0x00, 0x00);
	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

	sx126x_set_buffer_base_address(&sx126x_hal, 0x00, 0x00);
	sx126x_write_buffer(&sx126x_hal, 0x00, LSBuffer, payloadOutSize);

	if (tx_func != NULL)
	{
		tx_func();
	}
	sx126x_set_tx_with_timeout_in_rtc_step(&sx126x_hal, 0xfffff0);

	/* dynamically fill hopping table */
	uint16_t l = 0;

	/* wait for 4 hops to have been processed */
	if (params.hopping) {
		if (currenthop < nbBlock) {
			while (l < 4 ) {
				while(!(get_irq_status() & (SX126X_IRQ_LR_FHSS_HOP))) { delay(5); }
				sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);
				l++;
			}
		}

		l = 0; // set to start to table

		while (currenthop < nbBlock) {

			while(!(get_irq_status() & (SX126X_IRQ_LR_FHSS_HOP))) { delay(5); }

			sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

			uint32_t freq = refFreq_pll - freqHopping[currenthop] * grid_pll_steps;

			if (blockSize>nbBits)
				blockSize = nbBits;

	   		uint8_t data[] = { (uint8_t)((blockSize >> 8) & 0xff), (uint8_t)(blockSize & 0xff),
					           (uint8_t)((freq >> 24) & 0xff), (uint8_t)((freq >> 16) & 0xff), (uint8_t)((freq >> 8) & 0xff), (uint8_t)(freq & 0xff) };

			sx126x_write_register_bulk(&sx126x_hal, SX126X_LR_FHSS_REG_NUM_SYMBOLS_0 + l * 6, data, sizeof data);

			nbBits -= blockSize;
			currenthop++;
			l++;
			if (l>15) { l = 0;}
		}
	}

	/* Wait until TxDone bit set in IRQ status */
	while(!(get_irq_status() & SX126X_IRQ_TX_DONE))
	{
		delay(5);
	}
	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

	uint8_t lr_fhss_reg_value = 0;
	sx126x_write_register(&sx126x_hal, SX126X_LR_FHSS_REG_CTRL, &lr_fhss_reg_value, 1);
	
	// sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START);

	return RFT_STATUS_OK;
}

rft_status_t rfthings_sx126x::send_lorawan_over_lrfhss(byte *payload, uint32_t payload_len)
{
	return (this->send_lorawan_over_lrfhss(payload, payload_len, NULL));
}

rft_status_t rfthings_sx126x::send_lorawan_over_lrfhss(byte *payload, uint32_t payload_len, void (*tx_func)())
{
	// buld LoRaWAN packet
	unsigned char lorawan_packet[9 + 255 + 4];

	uint8_t packet_len = build_uplink_packet(payload, payload_len, lorawan_packet);

	// send LR-FHSS packet
	rft_status_t status = send_lrfhss(lorawan_packet, packet_len, NULL);
	lorawan_params.framecounter_uplink++;

	return status;
}

rft_status_t rfthings_sx126x::send_uplink(byte *payload, uint32_t payload_len, void (*tx_func)(), void (*rx_func)(), bool send_to_relay)
{
	// buld LoRaWAN packet
	unsigned char lorawan_packet[9 + 255 + 4];

	uint8_t packet_len = build_uplink_packet(payload, payload_len, lorawan_packet);

	// send LoRa

	if (send_lora(lorawan_packet, packet_len, 2000, tx_func) == RFT_STATUS_TX_TIMEOUT)
	{
		return RFT_STATUS_TX_TIMEOUT;
	}

	lorawan_params.framecounter_uplink++;

	if (this->lora_params.send_to_relay)
	{
		// No need downlink for this mode
		return RFT_STATUS_OK;
	}

	// receive downlink
	bool receive_downlink = false;

	sx126x_irq_mask_t irq_status;

	digitalWrite(sx126x_hal.antenna_switch, HIGH);
	if (rx_func != NULL)
	{
		rx_func();
	}

	// RX1 window
	sx126x_pkt_params_lora_t lora_pkt_params;
	lora_pkt_params.preamble_len_in_symb = 8;
	lora_pkt_params.header_type = SX126X_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = 255;
	lora_pkt_params.crc_is_on = true;
	lora_pkt_params.invert_iq_is_on = true;
	sx126x_set_lora_pkt_params(&sx126x_hal, &lora_pkt_params);

	sx126x_cfg_rx_boosted(&sx126x_hal, true);

	sx126x_set_rf_freq(&sx126x_hal, lorawan_params.rx1_frequency);

	sx126x_set_dio_irq_params(&sx126x_hal, 0x0fff, 0x0fff, 0x00, 0x00);

	sx126x_set_buffer_base_address(&sx126x_hal, 0x00, 0x00);

	delay(lorawan_params.rx1_delay - 200);

	sx126x_set_rx(&sx126x_hal, 500);

	while (1)
	{
		sx126x_get_irq_status(&sx126x_hal, &irq_status);
		if (irq_status & SX126X_IRQ_RX_DONE)
		{
			break;
		}
		if (irq_status & SX126X_IRQ_TIMEOUT)
		{
			break;
		}
	}

	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

	if (irq_status & SX126X_IRQ_RX_DONE)
	{
		receive_downlink = true;
	}

	if (!receive_downlink)
	{
		// RX2 window
		sx126x_mod_params_lora_t lora_mod_params;
		lora_mod_params.sf = map_spreading_factor(lorawan_params.rx2_spreading_factor);
		lora_mod_params.bw = map_bandwidth(lorawan_params.rx2_bandwidth);
		lora_mod_params.cr = map_coding_rate(lora_params.coding_rate);
		lora_mod_params.ldro = compute_lora_ldro();
		sx126x_set_lora_mod_params(&sx126x_hal, &lora_mod_params);

		sx126x_set_rf_freq(&sx126x_hal, lorawan_params.rx2_frequency);

		delay(800);

		sx126x_set_rx(&sx126x_hal, 500);

		while (1)
		{
			sx126x_get_irq_status(&sx126x_hal, &irq_status);
			if (irq_status & SX126X_IRQ_RX_DONE)
			{
				break;
			}
			if (irq_status & SX126X_IRQ_TIMEOUT)
			{
				break;
			}
		}

		sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

		if (irq_status & SX126X_IRQ_RX_DONE)
		{
			receive_downlink = true;
		}
	}

	digitalWrite(sx126x_hal.antenna_switch, LOW);

	// parse downlink
	if (!receive_downlink)
	{
		// sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START);
		lorawan_params.rx_length = 0;
		return RFT_STATUS_RX_TIMEOUT;
	}

	sx126x_pkt_status_lora_t pkt_status;
	sx126x_get_lora_pkt_status(&sx126x_hal, &pkt_status);

	lora_params.rssi = pkt_status.rssi_pkt_in_dbm;
	lora_params.snr = pkt_status.snr_pkt_in_db;
	lora_params.signal_rssi = pkt_status.signal_rssi_pkt_in_dbm;

	sx126x_rx_buffer_status_t rx_buffer_status;
	sx126x_get_rx_buffer_status(&sx126x_hal, &rx_buffer_status);

	payload_len = rx_buffer_status.pld_len_in_bytes;
	sx126x_read_buffer(&sx126x_hal, rx_buffer_status.buffer_start_pointer, payload, payload_len);

	// sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START);

	return parse_downlink(payload, payload_len);
}

rft_status_t rfthings_sx126x::send_join_request(void (*tx_func)(), void (*rx_func)())
{
	// buld join request
	unsigned char lorawan_packet[9 + 255 + 4];

	uint8_t packet_len = build_join_request(lorawan_packet);

	// send lora
	if (send_lora(lorawan_packet, packet_len, 2000, tx_func) == RFT_STATUS_TX_TIMEOUT)
	{
		return RFT_STATUS_TX_TIMEOUT;
	}

	// receive downlink
	bool receive_downlink = false;

	uint16_t irq_status;

	digitalWrite(sx126x_hal.antenna_switch, HIGH);
	if (rx_func != NULL)
	{
		rx_func();
	}

	// RX1 window
	sx126x_pkt_params_lora_t lora_pkt_params;
	lora_pkt_params.preamble_len_in_symb = 8;
	lora_pkt_params.header_type = SX126X_LORA_PKT_EXPLICIT;
	lora_pkt_params.pld_len_in_bytes = 255;
	lora_pkt_params.crc_is_on = true;
	lora_pkt_params.invert_iq_is_on = true;
	sx126x_set_lora_pkt_params(&sx126x_hal, &lora_pkt_params);

	sx126x_cfg_rx_boosted(&sx126x_hal, true);

	sx126x_set_rf_freq(&sx126x_hal, lorawan_params.rx1_frequency);

	sx126x_set_dio_irq_params(&sx126x_hal, 0x0fff, 0x0fff, 0x00, 0x00);

	sx126x_set_buffer_base_address(&sx126x_hal, 0x00, 0x00);

	delay(lorawan_params.rx1_delay - 200);

	sx126x_set_rx(&sx126x_hal, 500);

	while (1)
	{
		sx126x_get_irq_status(&sx126x_hal, &irq_status);
		if (irq_status & SX126X_IRQ_RX_DONE)
		{
			break;
		}
		if (irq_status & SX126X_IRQ_TIMEOUT)
		{
			break;
		}
	}

	sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

	if (irq_status & SX126X_IRQ_RX_DONE)
	{
		receive_downlink = true;
	}

	if (!receive_downlink)
	{
		// RX2 window
		sx126x_mod_params_lora_t lora_mod_params;
		lora_mod_params.sf = map_spreading_factor(lorawan_params.rx2_spreading_factor);
		lora_mod_params.bw = map_bandwidth(lorawan_params.rx2_bandwidth);
		lora_mod_params.cr = map_coding_rate(lora_params.coding_rate);
		lora_mod_params.ldro = compute_lora_ldro();
		sx126x_set_lora_mod_params(&sx126x_hal, &lora_mod_params);

		sx126x_set_rf_freq(&sx126x_hal, lorawan_params.rx2_frequency);

		delay(800);

		sx126x_set_rx(&sx126x_hal, 500);

		while (1)
		{
			sx126x_get_irq_status(&sx126x_hal, &irq_status);
			if (irq_status & SX126X_IRQ_RX_DONE)
			{
				break;
			}
			if (irq_status & SX126X_IRQ_TIMEOUT)
			{
				break;
			}
		}

		sx126x_clear_irq_status(&sx126x_hal, SX126X_IRQ_ALL);

		if (irq_status & SX126X_IRQ_RX_DONE)
		{
			receive_downlink = true;
		}
	}

	digitalWrite(sx126x_hal.antenna_switch, LOW);

	// parse downlink
	if (!receive_downlink)
	{
		// sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START);
		lorawan_params.rx_length = 0;
		return RFT_STATUS_RX_TIMEOUT;
	}

	sx126x_pkt_status_lora_t pkt_status;
	sx126x_get_lora_pkt_status(&sx126x_hal, &pkt_status);

	lora_params.rssi = pkt_status.rssi_pkt_in_dbm;
	lora_params.snr = pkt_status.snr_pkt_in_db;
	lora_params.signal_rssi = pkt_status.signal_rssi_pkt_in_dbm;

	sx126x_rx_buffer_status_t rx_buffer_status;
	sx126x_get_rx_buffer_status(&sx126x_hal, &rx_buffer_status);

	packet_len = rx_buffer_status.pld_len_in_bytes;
	sx126x_read_buffer(&sx126x_hal, rx_buffer_status.buffer_start_pointer, lorawan_packet, packet_len);

	// sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START);

	return parse_join_accept(lorawan_packet, packet_len);
}

rft_status_t rfthings_sx126x::config_continous_wave(void)
{
	return RFT_STATUS_ERROR_HARDWARE; // NOT IMPLEMENTED
}

rft_status_t rfthings_sx126x::start_continuous_wave(void)
{
	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC); // setStandby(0);

	sx126x_pa_cfg_params_t pa_config;
	pa_config.pa_duty_cycle = 0x04;
	pa_config.hp_max = 0x07;
	pa_config.device_sel = 0x00;
	pa_config.pa_lut = 0x01;
	sx126x_set_pa_cfg(&sx126x_hal, &pa_config);

	sx126x_set_tx_params(&sx126x_hal, lora_params.tx_power, SX126X_RAMP_10_US);
	sx126x_set_rf_freq(&sx126x_hal, lora_params.frequency);
	
	sx126x_set_tx_cw(&sx126x_hal);

	return RFT_STATUS_OK;
}

rft_status_t rfthings_sx126x::stop_continuous_wave(void)
{
	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC); // setStandby(0);
	// sx126x_set_sleep(&sx126x_hal, SX126X_SLEEP_CFG_WARM_START); // setSleep();
	return RFT_STATUS_OK;
}

rft_status_t rfthings_sx126x::sweep_continuous_wave(uint32_t start_freq, uint32_t stop_freq, uint32_t step, uint16_t duration)
{
	return RFT_STATUS_ERROR_HARDWARE; // NOT IMPLEMENTED
}

void rfthings_sx126x::calibrate_image(uint32_t frequency)
{
	uint8_t calFreq[2];
	calFreq[0] = 0xD7;
	calFreq[1] = 0xDB;

	if (lora_params.frequency > 900000000)
	{
		calFreq[0] = 0xE1;
		calFreq[1] = 0xE9;
	}
	else if (lora_params.frequency > 850000000)
	{
		calFreq[0] = 0xD7;
		calFreq[1] = 0xDB;
	}
	else if (lora_params.frequency > 770000000)
	{
		calFreq[0] = 0xC1;
		calFreq[1] = 0xC5;
	}
	else if (lora_params.frequency > 460000000)
	{
		calFreq[0] = 0x75;
		calFreq[1] = 0x81;
	}
	else if (lora_params.frequency > 425000000)
	{
		calFreq[0] = 0x6B;
		calFreq[1] = 0x6F;
	}

	sx126x_cal_img(&sx126x_hal, calFreq[0], calFreq[1]);
}

sx126x_lora_sf_t rfthings_sx126x::map_spreading_factor(rft_lora_spreading_factor_t spreading_factor)
{
	switch (spreading_factor)
	{
	case RFT_LORA_SPREADING_FACTOR_5:
		return SX126X_LORA_SF5;
		break;
	case RFT_LORA_SPREADING_FACTOR_6:
		return SX126X_LORA_SF6;
		break;
	case RFT_LORA_SPREADING_FACTOR_7:
		return SX126X_LORA_SF7;
		break;
	case RFT_LORA_SPREADING_FACTOR_8:
		return SX126X_LORA_SF8;
		break;
	case RFT_LORA_SPREADING_FACTOR_9:
		return SX126X_LORA_SF9;
		break;
	case RFT_LORA_SPREADING_FACTOR_10:
		return SX126X_LORA_SF10;
		break;
	case RFT_LORA_SPREADING_FACTOR_11:
		return SX126X_LORA_SF11;
		break;
	case RFT_LORA_SPREADING_FACTOR_12:
		return SX126X_LORA_SF12;
		break;
	default:
		return SX126X_LORA_SF7;
		break;
	}
}

sx126x_lora_bw_t rfthings_sx126x::map_bandwidth(rft_lora_bandwidth_t bandwidth)
{
	switch (bandwidth)
	{
	case RFT_LORA_BANDWIDTH_10KHZ:
		return SX126X_LORA_BW_010;
		break;
	case RFT_LORA_BANDWIDTH_15KHZ:
		return SX126X_LORA_BW_015;
		break;
	case RFT_LORA_BANDWIDTH_20KHZ:
		return SX126X_LORA_BW_020;
		break;
	case RFT_LORA_BANDWIDTH_31KHZ:
		return SX126X_LORA_BW_031;
		break;
	case RFT_LORA_BANDWIDTH_41KHZ:
		return SX126X_LORA_BW_041;
		break;
	case RFT_LORA_BANDWIDTH_62KHZ:
		return SX126X_LORA_BW_062;
		break;
	case RFT_LORA_BANDWIDTH_125KHZ:
		return SX126X_LORA_BW_125;
		break;
	case RFT_LORA_BANDWIDTH_250KHZ:
		return SX126X_LORA_BW_250;
		break;
	case RFT_LORA_BANDWIDTH_500KHZ:
		return SX126X_LORA_BW_500;
		break;
	default:
		return SX126X_LORA_BW_125;
		break;
	}
}

sx126x_lora_cr_t rfthings_sx126x::map_coding_rate(rft_lora_coding_rate_t coding_rate)
{
	switch (coding_rate)
	{
	case RFT_LORA_CODING_RATE_4_5:
		return SX126X_LORA_CR_4_5;
		break;
	case RFT_LORA_CODING_RATE_4_6:
		return SX126X_LORA_CR_4_6;
		break;
	case RFT_LORA_CODING_RATE_4_7:
		return SX126X_LORA_CR_4_7;
		break;
	case RFT_LORA_CODING_RATE_4_8:
		return SX126X_LORA_CR_4_8;
		break;
	default:
		return SX126X_LORA_CR_4_5;
		break;
	}
}

uint8_t rfthings_sx126x::compute_lora_ldro(void)
{
	switch (lora_params.bandwidth)
	{
	case RFT_LORA_BANDWIDTH_500KHZ:
		return 0x00;
		break;
	case RFT_LORA_BANDWIDTH_250KHZ:
		if (lora_params.spreading_factor == RFT_LORA_SPREADING_FACTOR_12)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
		break;
	case RFT_LORA_BANDWIDTH_125KHZ:
		if (lora_params.spreading_factor >= RFT_LORA_SPREADING_FACTOR_11)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
		break;
	case RFT_LORA_BANDWIDTH_62KHZ:
		if (lora_params.spreading_factor >= RFT_LORA_SPREADING_FACTOR_10)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
		break;
	case RFT_LORA_BANDWIDTH_41KHZ:
		if (lora_params.spreading_factor >= RFT_LORA_SPREADING_FACTOR_9)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
		break;
	case RFT_LORA_BANDWIDTH_31KHZ:
	case RFT_LORA_BANDWIDTH_20KHZ:
	case RFT_LORA_BANDWIDTH_15KHZ:
	case RFT_LORA_BANDWIDTH_10KHZ:
		return 0x01;
		break;
	default:
		return 0x00;
		break;
	}
}

uint8_t rfthings_sx126x::compute_lora_ldro(rft_lora_params_t lora_params)
{
	switch (lora_params.bandwidth)
	{
	case RFT_LORA_BANDWIDTH_500KHZ:
		return 0x00;
		break;
	case RFT_LORA_BANDWIDTH_250KHZ:
		if (lora_params.spreading_factor == RFT_LORA_SPREADING_FACTOR_12)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
		break;
	case RFT_LORA_BANDWIDTH_125KHZ:
		if (lora_params.spreading_factor >= RFT_LORA_SPREADING_FACTOR_11)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
		break;
	case RFT_LORA_BANDWIDTH_62KHZ:
		if (lora_params.spreading_factor >= RFT_LORA_SPREADING_FACTOR_10)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
		break;
	case RFT_LORA_BANDWIDTH_41KHZ:
		if (lora_params.spreading_factor >= RFT_LORA_SPREADING_FACTOR_9)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
		break;
	case RFT_LORA_BANDWIDTH_31KHZ:
	case RFT_LORA_BANDWIDTH_20KHZ:
	case RFT_LORA_BANDWIDTH_15KHZ:
	case RFT_LORA_BANDWIDTH_10KHZ:
		return 0x01;
		break;
	default:
		return 0x00;
		break;
	}
}

uint16_t rfthings_sx126x::get_random_number(void)
{
	uint16_t buf = 0;
	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC); // setStandby(0);
	delay(1);
	// Set radio in continuous reception
	sx126x_set_rx(&sx126x_hal, 0); // setRx( 0 );
	delay(1);
	sx126x_read_register(&sx126x_hal, SX126X_REG_RNGBASEADDRESS, (uint8_t *)&buf, 2); // buf = readRegister16(SX126X_REG_RNGBASEADDRESS);
	sx126x_set_standby(&sx126x_hal, SX126X_STANDBY_CFG_RC);														// setStandby(0);
	return buf;
}

uint16_t rfthings_sx126x::get_irq_status(void)
{
	sx126x_irq_mask_t irqMask;
	sx126x_status_t status = sx126x_get_irq_status(&sx126x_hal, &irqMask);

	if (status == RFT_STATUS_OK)
	{
			return ((uint16_t) irqMask);
	}
	else
	{
		return 0;
	}
}
