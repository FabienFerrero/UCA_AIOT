# 6. Continuous wave

## Description

This section is intended for antenna or system functional testing. The sketch in this section generates a continuous wave at a specific **Frequency** and **Output power level**.

These 2 following function is used for changing the continuous wave parameters:

```
// CW Configuration
sx126x.set_frequency(868100000);
sx126x.set_tx_power(14);
```

To start the continuous wave, use this following function:

```
sx126x.start_continuous_wave();
```

To stop the continuous wave, use this following function:

```
sx126x.stop_continuous_wave();
```

# Function Description

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

### Start CW

This is a non-blocking function to command the LoRa Module to start the continuous wave.

```
rft_status_t start_continuous_wave(void)
```

### Stop CW

This is a non-blocking function to command the LoRa Module to stop the continuous wave.

```
rft_status_t stop_continuous_wave(void)
```

##### Maintained by Prof. F. Fererro & mtnguyen
