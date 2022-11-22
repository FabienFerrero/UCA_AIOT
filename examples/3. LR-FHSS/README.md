# 3. LR-FHSS

## Description

Examples in this section demonstrate sending LR-FHSS packets with or without LoRaWAN packet format. Every `TX_INTERVAL` seconds, a LR-FHSS packet is sent. The library supports uplink LR-FHSS communication only.

## lr_fhss_raw_example

The parameters of LR-FHSS can be modify via following functions:

```
sx126x.set_lrfhss_codingRate(RFT_LRFHSS_CODING_RATE_1_3);
sx126x.set_lrfhss_bandwidth(RFT_LRFHSS_BANDWIDTH_335_9_KHZ);
sx126x.set_lrfhss_grid(RFT_LRFHSS_GRID_3_9_KHZ);
sx126x.set_lrfhss_hopping(true);
sx126x.set_lrfhss_nbSync(4);
sx126x.set_lrfhss_frequency(862750000);
sx126x.set_lrfhss_power(21);
```

## lr_fhss_lorawan_example

This sketch operates the same as the **lr_fhss_raw_example** sketch. Except for the LoRaWAN packet format. Thus, it requires **Device address** (dev_addr), **Network Session Key** (NwkSKey) and **Application Session Key** (AppSKey) to operate. Replace those information in the source code with the ones received from your LoRaWAN Network Server. Make sure that they are all in MSB format.

If you are working with The Things Network, refer to this [Adding Devices](https://www.thethingsindustries.com/docs/devices/adding-devices/) instruction.

```
static uint8_t nwkS_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t appS_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t dev_addr[] = {0x00, 0x00, 0x00, 0x00};
```

The parameters of LoRaWAN can be modify via following functions:

```
sx126x.set_tx_port(1);
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

(*LoRaWAN 1.0.3 Specification*)

### Set LR-FHSS Transmit Power
```
void set_lrfhss_power(int8_t power);
```

| Parameter | Value                                   | Description                                       |
|:---------:|:---------------------------------------:|:-------------------------------------------------:|
|  power    | **-9** (**0xF7**) to **+22** (**0x16**) | The TX output power in dBm with the step of 1 dBm |

### Set LR-FHSS Transmit Frequency
```
void set_lrfhss_frequency(uint32_t frequency);
```

| Parameter |       Description      |
|:---------:|:----------------------:|
| frequency | The TX frequency in Hz |

### Set LR-FHSS Transmit Hopping
```
void set_lrfhss_hopping(bool hopping);
```

| Parameter | Value        | Description                |
|:---------:|:------------:|:--------------------------:|
|  hopping  | true / false | Enable hopping for LR-FHSS |

### Set number of LR-FHSS header blocks
```
void set_lrfhss_nbSync(uint8_t nbSync);
```

| Parameter |       Description               |
|:---------:|:-------------------------------:|
| nbSync    | Number of LR-FHSS header blocks |

### Set LR-FHSS Transmit Bandwidth
```
void set_lrfhss_bandwidth(rft_lrfhss_bandwidth_t bandwidth);
```

| Parameter | Value                               | Description           |
|:---------:|:-----------------------------------:|:---------------------:|
| bandwidth | **RFT_LRFHSS_BANDWIDTH_39_06_KHZ**  | Bandwidth 39.06 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_89_84_KHZ**  | Bandwidth 89.84 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_85_94_KHZ**  | Bandwidth 85.94 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_136_7_KHZ**  | Bandwidth 136.7 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_187_5_KHZ**  | Bandwidth 187.5 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_183_5_KHZ**  | Bandwidth 183.5 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_335_9_KHZ**  | Bandwidth 335.9 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_386_7_KHZ**  | Bandwidth 386.7 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_722_6_KHZ**  | Bandwidth 722.6 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_710_9_KHZ**  | Bandwidth 710.9 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_773_4_KHZ**  | Bandwidth 773.4 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_761_7_KHZ**  | Bandwidth 761.7 kHz   |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_1523_4_KHZ** | Bandwidth 1523.4 kHz  |
| bandwidth | **RFT_LRFHSS_BANDWIDTH_1574_2_KHZ** | Bandwidth 1574.2 kHz  |

### Set LR-FHSS Transmit Coding Rate
```
void set_lrfhss_codingRate(rft_lrfhss_coding_rate_t coding_rate);
```

| Parameter   | Value                           | Description     |
|:-----------:|:-------------------------------:|:---------------:|
| coding_rate | **RFT_LRFHSS_CODING_RATE_5_6**  | Coding Rate 5/6 |
| coding_rate | **RFT_LRFHSS_CODING_RATE_2_3**  | Coding Rate 2/3 |
| coding_rate | **RFT_LRFHSS_CODING_RATE_1_2**  | Coding Rate 1/2 |
| coding_rate | **RFT_LRFHSS_CODING_RATE_1_3**  | Coding Rate 1/3 |

### Set LR-FHSS Transmit Grid
```
void set_lrfhss_grid(rft_lrfhss_grid_t grid);
```

| Parameter | Value                         | Description   |
|:---------:|:-----------------------------:|:-------------:|
| grid      | **RFT_LRFHSS_GRID_25_KHZ**    | Grid 25 kHz   |
| grid      | **RFT_LRFHSS_GRID_3_9_KHZ**   | Grid 3.9 kHz  |

##### Maintained by Prof. F. Ferrero & mtnguyen
