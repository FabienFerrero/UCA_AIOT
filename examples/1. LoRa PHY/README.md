# 1. LoRa PHY

## Description

Examples in this section demonstrate a simple device-to-device communication over LoRa PHY. The **lora_phy_sender_example** send a LoRa PHY packet every `TX_INTERVAL` seconds. The **lora_phy_receiver_example** listen continuously for LoRa PHY packets and print packet's information to serial monitor.

## lora_phy_sender_example
## lora_phy_receiver_example

The parameters of LoRa PHY can be modify via following functions:

```
sx126x.set_tx_power(14);
sx126x.set_frequency(868100000);
sx126x.set_spreading_factor(RFT_LORA_SPREADING_FACTOR_7);
sx126x.set_bandwidth(RFT_LORA_BANDWIDTH_125KHZ);
sx126x.set_coding_rate(RFT_LORA_CODING_RATE_4_5);
sx126x.set_syncword(RFT_LORA_SYNCWORD_PUBLIC);
```

### Set LoRa Transmit Power
```
void set_tx_power(int8_t tx_power);
```

(*Not available/needed for receivers*)

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
