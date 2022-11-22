# 2. LoRaWAN

## Description

The example in this section demonstrate sending simple LoRaWAN packets in Activation by Personalization (ABP) activation. The **lorawan_abp_example** send a *"hello world"* packet every `TX_INTERVAL` seconds to the Gateway.

## lorawan_abp_example

To be able to send message to LoRaWAN Network Server (e.g. The Things Network) in ABP activation, the device need 3 informations including **Device address** (dev_addr), **Network Session Key** (NwkSKey), **Application Session Key** (AppSKey). Those 3 information can be retrived when doing registration on your LoRaWAN Network Server. Replace **dev_addr**, **nwkS_key** & **appS_key** in the example source code with received information. Make sure that they are all in MSB format.

If you are working with The Things Network, refer to this [Adding Devices](https://www.thethingsindustries.com/docs/devices/adding-devices/) instruction.

```
static uint8_t nwkS_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t appS_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t dev_addr[] = {0x00, 0x00, 0x00, 0x00};
```

The parameters of LoRaWAN can be modify via following functions:

```
sx126x.set_tx_port(1);
sx126x.set_rx1_delay(1000);
```

The parameters of LoRa PHY can be modify via following functions:

```
sx126x.set_tx_power(14);
sx126x.set_frequency(868100000);
sx126x.set_spreading_factor(RFT_LORA_SPREADING_FACTOR_7);
sx126x.set_bandwidth(RFT_LORA_BANDWIDTH_125KHZ);
sx126x.set_coding_rate(RFT_LORA_CODING_RATE_4_5);
sx126x.set_syncword(RFT_LORA_SYNCWORD_PUBLIC);
```

# Function Description

### Set LoRaWAN Transmit fPort

```
void set_tx_port(uint8_t tx_port);
```

| Parameter |        Value        |               Description               |
|:---------:|:-------------------:|:---------------------------------------:|
| tx_port   | 1..223 (0x01..0xDF) | Application-specific port (Recommended) |
| tx_port   | 224                 | LoRaWAN Mac layer test protocol         |

([*LoRaWAN 1.0.3 Specification*](https://lora-alliance.org/resource_hub/lorawan-specification-v1-0-3/))

### Set LoRaWAN Downlink Windows 1 Delay

```
void set_rx1_delay(uint32_t rx1_delay);
```

| Parameter |               Description                          |
|:---------:|:--------------------------------------------------:|
| rx1_delay | Delay time for first downlink windows in second(s) |

([*RP2-1.0.3 LoRaWAN Regional Parameters*](https://lora-alliance.org/resource_hub/rp2-1-0-3-lorawan-regional-parameters/))

### Set LoRa Transmit Power
```
void set_tx_power(int8_t tx_power);
```

| Parameter | Value                                   | Description                                       |
|:---------:|:---------------------------------------:|:-------------------------------------------------:|
|  tx_power | **-9** (**0xF7**) to **+22** (**0x16**) | The TX output power in dBm with the step of 1 dBm |

### Set LoRa Transmit Frequency
```
void set_frequency(uint32_t frequency);
```

| Parameter |       Description      |
|:---------:|:----------------------:|
| frequency | The TX frequency in Hz |

### Set LoRa Transmit Spreading Factor
```
void set_spreading_factor(rft_lora_spreading_factor_t spreading_factor);
```

| Parameter        | Value                            | Description         |
|:----------------:|:--------------------------------:|:-------------------:|
| spreading_factor | **RFT_LORA_SPREADING_FACTOR_5**  | Spreading Factor 5  |
| spreading_factor | **RFT_LORA_SPREADING_FACTOR_6**  | Spreading Factor 6  |
| spreading_factor | **RFT_LORA_SPREADING_FACTOR_7**  | Spreading Factor 7  |
| spreading_factor | **RFT_LORA_SPREADING_FACTOR_8**  | Spreading Factor 8  |
| spreading_factor | **RFT_LORA_SPREADING_FACTOR_9**  | Spreading Factor 9  |
| spreading_factor | **RFT_LORA_SPREADING_FACTOR_10** | Spreading Factor 10 |
| spreading_factor | **RFT_LORA_SPREADING_FACTOR_11** | Spreading Factor 11 |
| spreading_factor | **RFT_LORA_SPREADING_FACTOR_12** | Spreading Factor 12 |

### Set LoRa Transmit Bandwidth
```
void set_bandwidth(rft_lora_bandwidth_t bandwidth);
```

| Parameter | Value                         | Description         |
|:---------:|:-----------------------------:|:-------------------:|
| bandwidth | **RFT_LORA_BANDWIDTH_10KHZ**  | Bandwidth 10.42 kHz |
| bandwidth | **RFT_LORA_BANDWIDTH_15KHZ**  | Bandwidth 15.63 kHz |
| bandwidth | **RFT_LORA_BANDWIDTH_20KHZ**  | Bandwidth 20.83 kHz |
| bandwidth | **RFT_LORA_BANDWIDTH_31KHZ**  | Bandwidth 31.25 kHz |
| bandwidth | **RFT_LORA_BANDWIDTH_41KHZ**  | Bandwidth 41.67 kHz |
| bandwidth | **RFT_LORA_BANDWIDTH_62KHZ**  | Bandwidth 62.50 kHz |
| bandwidth | **RFT_LORA_BANDWIDTH_125KHZ** |  Bandwidth 125 kHz  |
| bandwidth | **RFT_LORA_BANDWIDTH_250KHZ** |  Bandwidth 250 kHz  |
| bandwidth | **RFT_LORA_BANDWIDTH_500KHZ** |  Bandwidth 500 kHz  |

### Set LoRa Transmit Coding Rate
```
void set_coding_rate(rft_lora_coding_rate_t coding_rate);
```

| Parameter   | Value                         | Description     |
|:-----------:|:-----------------------------:|:---------------:|
| coding_rate | **RFT_LORA_CODING_RATE_4_5**  | Coding Rate 4/5 |
| coding_rate | **RFT_LORA_CODING_RATE_4_6**  | Coding Rate 4/5 |
| coding_rate | **RFT_LORA_CODING_RATE_4_7**  | Coding Rate 4/5 |
| coding_rate | **RFT_LORA_CODING_RATE_4_8**  | Coding Rate 4/5 |

### Set LoRa Transmit Syncword
```
void set_syncword(rft_lora_syncword_t syncword);
```

| Parameter | Value                         | Description                       |
|:---------:|:-----------------------------:|:---------------------------------:|
| syncword  | **RFT_LORA_SYNCWORD_PRIVATE** | Private Network Syncword (0x1424) |
| syncword  | **RFT_LORA_SYNCWORD_PUBLIC**  | Public Network Syncword (0x3444)  |

##### Maintained by Prof. F. Fererro & mtnguyen
