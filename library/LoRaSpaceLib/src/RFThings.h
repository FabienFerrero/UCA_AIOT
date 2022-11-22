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

#ifndef _RFTHINGS_H_
#define _RFTHINGS_H_

#include <Arduino.h>
#include "stdint.h"

// Region
typedef enum
{
	RFT_REGION_EU863_870,
	RFT_REGION_US902_928,
	RFT_REGION_CN470_510,
	RFT_REGION_AU915_928,
	RFT_REGION_AS920_923,
	RFT_REGION_AS923_925,
	RFT_REGION_KR920_923,
	RFT_REGION_IN865_867
} rft_region_t;

// Sync word
typedef uint16_t rft_lora_syncword_t;
#define RFT_LORA_SYNCWORD_PRIVATE (0x1424)
#define RFT_LORA_SYNCWORD_PUBLIC (0x3444)

// Coding rate (CR)
typedef enum
{
	RFT_LORA_CODING_RATE_4_5,
	RFT_LORA_CODING_RATE_4_6,
	RFT_LORA_CODING_RATE_4_7,
	RFT_LORA_CODING_RATE_4_8
} rft_lora_coding_rate_t;

// Spreading factor (SF)
typedef enum
{
	RFT_LORA_SPREADING_FACTOR_5,
	RFT_LORA_SPREADING_FACTOR_6,
	RFT_LORA_SPREADING_FACTOR_7,
	RFT_LORA_SPREADING_FACTOR_8,
	RFT_LORA_SPREADING_FACTOR_9,
	RFT_LORA_SPREADING_FACTOR_10,
	RFT_LORA_SPREADING_FACTOR_11,
	RFT_LORA_SPREADING_FACTOR_12
} rft_lora_spreading_factor_t;

// Bandwidth
typedef enum
{
	RFT_LORA_BANDWIDTH_10KHZ,
	RFT_LORA_BANDWIDTH_15KHZ,
	RFT_LORA_BANDWIDTH_20KHZ,
	RFT_LORA_BANDWIDTH_31KHZ,
	RFT_LORA_BANDWIDTH_41KHZ,
	RFT_LORA_BANDWIDTH_62KHZ,
	RFT_LORA_BANDWIDTH_125KHZ,
	RFT_LORA_BANDWIDTH_250KHZ,
	RFT_LORA_BANDWIDTH_500KHZ,
	RFT_LORA_BANDWIDTH_200KHZ, // Use for LR112X only
	RFT_LORA_BANDWIDTH_400KHZ, // Use for LR112X only
	RFT_LORA_BANDWIDTH_800KHZ	 // Use for LR112X only
} rft_lora_bandwidth_t;

// LoRa tx/rx parameters
typedef struct
{
	// TX parameter
	int8_t tx_power;
	rft_lora_spreading_factor_t spreading_factor;
	rft_lora_coding_rate_t coding_rate;
	rft_lora_bandwidth_t bandwidth;
	rft_lora_syncword_t syncword;

	// RX parameter
	int8_t snr;
	int16_t rssi;
	int16_t signal_rssi;

	// Frequency to send/reveive LoRa
	uint32_t frequency;

	// Use for Relay application
	bool send_to_relay;
	uint32_t relay_sleep_interval_us;		// Default value is 1E6 us (1 second)
	uint8_t relay_rx_symbol;						// Default value is 5 symbols
	uint8_t relay_max_rx_packet_length; // Default value is 120 bytes

	// For Doppler test
	bool force_ldro;

} rft_lora_params_t;

// LR-FHSS parameters
typedef enum {
	RFT_LRFHSS_CODING_RATE_5_6 = 0,
	RFT_LRFHSS_CODING_RATE_2_3 = 1,
	RFT_LRFHSS_CODING_RATE_1_2 = 2,
	RFT_LRFHSS_CODING_RATE_1_3 = 3
} rft_lrfhss_coding_rate_t;

typedef enum {
	RFT_LRFHSS_GRID_25_KHZ = 0,
	RFT_LRFHSS_GRID_3_9_KHZ = 1
} rft_lrfhss_grid_t;

typedef enum {
	RFT_LRFHSS_BANDWIDTH_39_06_KHZ = 0,
	RFT_LRFHSS_BANDWIDTH_89_84_KHZ = 1,
	RFT_LRFHSS_BANDWIDTH_85_94_KHZ = 1, // new (v1)
	RFT_LRFHSS_BANDWIDTH_136_7_KHZ = 2,
	RFT_LRFHSS_BANDWIDTH_187_5_KHZ = 3,
	RFT_LRFHSS_BANDWIDTH_183_5_KHZ = 3, // new (v1)
	RFT_LRFHSS_BANDWIDTH_335_9_KHZ = 4,
	RFT_LRFHSS_BANDWIDTH_386_7_KHZ = 5,
	RFT_LRFHSS_BANDWIDTH_722_6_KHZ = 6, // 3.9 khz
	RFT_LRFHSS_BANDWIDTH_710_9_KHZ = 6, // 25 kHz (v1)
	RFT_LRFHSS_BANDWIDTH_773_4_KHZ = 7, // 3.9 kHz
	RFT_LRFHSS_BANDWIDTH_761_7_KHZ = 7, // 25 kHz (v1)
	RFT_LRFHSS_BANDWIDTH_1523_4_KHZ = 8,
	RFT_LRFHSS_BANDWIDTH_1574_2_KHZ = 9
} rft_lrfhss_bandwidth_t;

typedef struct
{
	rft_lrfhss_coding_rate_t codingRate; /*!< Coding rate */
	rft_lrfhss_bandwidth_t bandwidth;	 /*!< Bandwidth */
	rft_lrfhss_grid_t grid;						 /*!< Grid */
	bool hopping;									 /*!< Frequency hopping on or off. Note that frequency hopping is not supported on the SX1276, and this parameter will be ignored */
	uint8_t nbSync;								 /*!< Number of header blocks */
	uint32_t frequency;						 /*!< Transmission frequency, in Hz */
	int8_t power;									 /*!< Transmission power */
} rft_lrfhss_params_t;

// LoraWAN activation types
typedef enum
{
	RFT_LORAWAN_ACTIVATION_TYPE_OTAA,
	RFT_LORAWAN_ACTIVATION_TYPE_ABP
} rft_lorawan_activation_type_t;

// LoRaWan uplink frequency
/*
#if REGION == R_EU863_870
	uint32_t rft_lorawan_uplink_frequency[8] = {
		868100000,
		868300000,
		868500000,
		867100000,
		867300000,
		867500000,
		867700000,
		867900000
	};
#elif REGION == R_US902_928
	uint32_t rft_lorawan_uplink_frequency[8] = {
		903900000,
		904100000,
		904300000,
		904500000,
		904700000,
		904900000,
		905100000,
		905300000
	};
#elif REGION == R_CN470_510
	uint32_t rft_lorawan_uplink_frequency[8] = {
		486300000,
		486500000,
		486700000,
		486900000,
		487100000,
		487300000,
		487500000,
		487700000
	};
#elif REGION == R_AU915_928
	uint32_t rft_lorawan_uplink_frequency[9] = {
		916800000,
		917000000,
		917200000,
		917400000,
		917600000,
		917800000,
		918000000,
		918200000,
		917500000
	};
#elif REGION == R_AS920_923
	uint32_t rft_lorawan_uplink_frequency[9] = {
		923200000,
		923400000,
		922200000,
		922400000,
		922600000,
		922800000,
		923000000,
		922000000,
		922100000
	};
#elif REGION == R_AS923_925
	uint32_t rft_lorawan_uplink_frequency[9] = {
		923200000,
		923400000,
		923600000,
		923800000,
		924000000,
		924200000,
		924400000,
		924600000,
		924500000
	};
#elif REGION == R_KR920_923
	uint32_t rft_lorawan_uplink_frequency[7] = {
		922100000,
		922300000,
		922500000,
		922700000,
		922900000,
		923100000,
		923300000
	};
#elif REGION == R_IN865_867
	uint32_t rft_lorawan_uplink_frequency[3] = {
		865062500,
		865402500,
		865985000
	};
#endif
*/

// LoraWAN framecounter size
typedef uint8_t rft_lorawan_framecounter_size_t;
#define RFT_LORAWAN_FRAMECOUNTER_SIZE_16 (16)
#define RFT_LORAWAN_FRAMECOUNTER_SIZE_32 (32)

// LoRaWAN parameters
typedef struct
{
	rft_lorawan_activation_type_t activation_type;

	// ABP activaion information
	uint8_t device_address[4];
	uint8_t network_session_key[16];
	uint8_t application_session_key[16];

	// OTAA activaion information
	uint8_t devive_eui[8];
	uint8_t application_eui[8];
	uint8_t application_key[16];
	uint8_t dev_nonce[2];

	// Downlink modulation param
	uint32_t rx1_delay;
	uint32_t rx1_frequency;
	uint32_t rx2_frequency;
	rft_lora_bandwidth_t rx2_bandwidth;
	rft_lora_spreading_factor_t rx2_spreading_factor;

	rft_lorawan_framecounter_size_t framecounter_size;
	uint32_t framecounter_uplink;
	uint32_t framecounter_downlink;
	uint8_t tx_port;
	uint8_t rx_port;
	uint8_t rx_length;
} rft_lorawan_params_t;

// RFThings status code
typedef enum
{
	RFT_STATUS_OK,
	RFT_STATUS_TX_DONE,
	RFT_STATUS_RX_DONE,
	RFT_STATUS_WRONG_MIC,
	RFT_STATUS_TX_TIMEOUT,
	RFT_STATUS_RX_TIMEOUT,
	RFT_STATUS_JOIN_ACCEPTED,
	RFT_STATUS_ERROR_HARDWARE,
	RFT_STATUS_ERROR_INVALID_PARAM,
	RFT_STATUS_ERROR_INVALID_REGION,
	RFT_STATUS_PREAMBLE_DETECT_FAIL,
	RFT_STATUS_ERROR_PAYLOAD_TOO_LONG,
	RFT_STATUS_ERROR_WRONG_LR11XX_FIRMWARE_VERSION,
	RFT_STATUS_ERROR_INAVLID_LORAWAN_ACTIVATION_TYPE
} rft_status_t;

const char *rft_status_to_str(rft_status_t status);
void worship(void);

#endif
