# 3. Relay

## Description

Examples in this section demonstrate a simple LoRaWAN Relay application.

**sensor_to_relay_example** sends a "*Hello world*" LoRaWAN packet with 1-second extended preamble length every `TX_INTERVAL` seconds.

**relay_example** continuously listens for LoRaWAN packets from sensors (or other LoRaWAN device) & prints packet information to Serial Monitor if it's successfully received. This sketch is **NOT** included packet integrity check, packet sender address filter, and repeat the received packet.

## sensor_to_relay_example

This sketch is basically a LoRaWAN ABP example with the extended preamble length. All functions to modify LoRaWAN parameters are the same as this [**#lorawan_abp_example**](./../2.%20LoRaWAN/README.md#lorawan_abp_example) section. The extended preamble length can be enable with the following function:

```
sx126x.set_send_to_relay(true);
```

## relay_example

The relay receives only packets from devices/sensors that matches LoRaWAN parameters. Listening LoRaWAN parameters is listed in a struct `rft_lora_params_t relay_lora_params` as follow:

```
relay_lora_params.frequency = 866600000;
relay_lora_params.spreading_factor = RFT_LORA_SPREADING_FACTOR_7;
relay_lora_params.bandwidth = RFT_LORA_BANDWIDTH_125KHZ;
relay_lora_params.coding_rate = RFT_LORA_CODING_RATE_4_5;
relay_lora_params.syncword = RFT_LORA_SYNCWORD_PUBLIC;
```

This struct `rft_lora_params_t relay_lora_params` is put into `relay` function later. This `relay` function is a blocking function. It's looped infinitely until detect a preamble (Channel Activity Detection).

```
sx126x.relay(&relay_lora_params, payload, payload_len, NULL, NULL);
```

The relay's Channel Activity Detection can be modified to archive `higher successful reception rate` or `lower power consumption`. It's highly recommended to NOT change these following paramters for best tested relay performance (measured by RFThings).

```
relay_lora_params.relay_sleep_interval_us = 1E6; // 1 second
relay_lora_params.relay_rx_symbol = 5;
relay_lora_params.relay_max_rx_packet_length = 120;
```

# Function Description

### Enable extended preamble length

```
void set_send_to_relay(bool _send_to_relay);
```

| Parameter        |        Value        |               Description                |
|:----------------:|:-------------------:|:----------------------------------------:|
| _send_to_relay   | true / false        | Enable 1-second extended preamble length |

### rft_lora_params_t relay_lora_params struct
```
rft_lora_params_t relay_lora_params;
```

|              Parameter                       |          Data type          |           Description          |
|:--------------------------------------------:|:---------------------------:|:------------------------------:|
| relay_lora_params.frequency                  | uint32_t                    | The TX frequency in Hz         |
| relay_lora_params.spreading_factor           | rft_lora_spreading_factor_t | LoRa Transmit Spreading Factor |
| relay_lora_params.bandwidth                  | rft_lora_bandwidth_t        | LoRa Transmit Bandwidth        |
| relay_lora_params.coding_rate                | rft_lora_coding_rate_t      | LoRa Transmit Coding Rate      |
| relay_lora_params.syncword                   | rft_lora_syncword_t         | LoRa Transmit Syncword         |
| relay_lora_params.send_to_relay              | bool                        | LoRa Transmit Syncword         |
| relay_lora_params.relay_sleep_interval_us    | uint32_t                    | Sleep interval between 2 wakeup windows |
| relay_lora_params.relay_rx_symbol            | uint8_t                     | The number of receive symbols to detect packet incoming in each wakeup windows |
| relay_lora_params.relay_max_rx_packet_length | uint8_t                     | The maximum number of receive packet length in byte(s) |

Details on value of each member of the struct are listed as follow:

| Parameter        | Value                            | Description         |
|:----------------:|:--------------------------------:|:-------------------:|
| relay_lora_params.spreading_factor | **RFT_LORA_SPREADING_FACTOR_5**  | Spreading Factor 5  |
| relay_lora_params.spreading_factor | **RFT_LORA_SPREADING_FACTOR_6**  | Spreading Factor 6  |
| relay_lora_params.spreading_factor | **RFT_LORA_SPREADING_FACTOR_7**  | Spreading Factor 7  |
| relay_lora_params.spreading_factor | **RFT_LORA_SPREADING_FACTOR_8**  | Spreading Factor 8  |
| relay_lora_params.spreading_factor | **RFT_LORA_SPREADING_FACTOR_9**  | Spreading Factor 9  |
| relay_lora_params.spreading_factor | **RFT_LORA_SPREADING_FACTOR_10** | Spreading Factor 10 |
| relay_lora_params.spreading_factor | **RFT_LORA_SPREADING_FACTOR_11** | Spreading Factor 11 |
| relay_lora_params.spreading_factor | **RFT_LORA_SPREADING_FACTOR_12** | Spreading Factor 12 |

| Parameter                   | Value                         | Description         |
|:---------------------------:|:-----------------------------:|:-------------------:|
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_10KHZ**  | Bandwidth 10.42 kHz |
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_15KHZ**  | Bandwidth 15.63 kHz |
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_20KHZ**  | Bandwidth 20.83 kHz |
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_31KHZ**  | Bandwidth 31.25 kHz |
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_41KHZ**  | Bandwidth 41.67 kHz |
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_62KHZ**  | Bandwidth 62.50 kHz |
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_125KHZ** |  Bandwidth 125 kHz  |
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_250KHZ** |  Bandwidth 250 kHz  |
| relay_lora_params.bandwidth | **RFT_LORA_BANDWIDTH_500KHZ** |  Bandwidth 500 kHz  |

| Parameter                     | Value                         | Description     |
|:-----------------------------:|:-----------------------------:|:---------------:|
| relay_lora_params.coding_rate | **RFT_LORA_CODING_RATE_4_5**  | Coding Rate 4/5 |
| relay_lora_params.coding_rate | **RFT_LORA_CODING_RATE_4_6**  | Coding Rate 4/5 |
| relay_lora_params.coding_rate | **RFT_LORA_CODING_RATE_4_7**  | Coding Rate 4/5 |
| relay_lora_params.coding_rate | **RFT_LORA_CODING_RATE_4_8**  | Coding Rate 4/5 |

| Parameter                   | Value                         | Description                       |
|:---------------------------:|:-----------------------------:|:---------------------------------:|
| relay_lora_params.syncword  | **RFT_LORA_SYNCWORD_PRIVATE** | Private Network Syncword (0x1424) |
| relay_lora_params.syncword  | **RFT_LORA_SYNCWORD_PUBLIC**  | Public Network Syncword (0x3444)  |

### Relay Function
```
rft_status_t relay(rft_lora_params_t *relay_lora_params, byte *payload, uint32_t &payload_len, void (*rx_func)(void), void (*sleep_func)(void));
```

| Parameter         |       Description      |
|:-----------------:|:----------------------:|
| relay_lora_params | Pointer to the rft_lora_params_t struct of LoRa configuration that the sender device |
| payload           | Pointer to the received packet buffer |
| payload_len       | The received packet length |
| rx_func           | Pointer to the receiving function. This function will be invoke before wakeup windows. It's normally used for setup RF Switch or Front-End. |
| sleep_func        | Pointer to the Micro-controller sleep function. If this is not NULL, the Micro-controller is in sleep mode until then relay `function` return. |

| Return value                    |       Description      |
|:-------------------------------:|:----------------------:|
| RFT_STATUS_OK                   | Packet received successfully |
| RFT_STATUS_PREAMBLE_DETECT_FAIL | Preamble detected but not detect Syncword |
| RFT_STATUS_RX_TIMEOUT           | Preamble & Syncword detected but Receiving takes too long |

##### Maintained by Prof. F. Ferrero & mtnguyen
