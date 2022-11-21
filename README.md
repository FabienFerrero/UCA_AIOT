# UCA_AIOT


![alt text](doc/Untitled.png)


## Installing Board Manager


Please use the official Arduino Core from RFThings repository to configure your Arduino IDE with RFThings AIoT Board: [RFThings/arduino-STM32L4](https://github.com/RFThings/arduino-STM32L4)

**Ensure that the installed core version is 0.0.61 (or above) to be compatible with all examples in this repository.**

## Schematic

The schematic of the board is available in [doc/LS200-007_SCHEMATIC.pdf](https://github.com/FabienFerrero/DKIOT/blob/main/doc/LS200-007_SCHEMATIC.pdf)

## Supported Library

The board has been successfully tested with several SX1262 library :

* LoRaWAN Mac protocol: [FabienFerrero/basicmac](https://github.com/FabienFerrero/basicmac) forked from: [LacunaSpace/basicmac](https://github.com/LacunaSpace/basicmac)

**Warning**: Add the following line in the code to make it work.

```
#define ARDUINO_STM32L4_LS200
```

* LoRa Physical layer: [StuartsProjects/SX12XX-LoRa](https://github.com/StuartsProjects/SX12XX-LoRa)

**Warning**: SPI PINS for SX1262 must be defined
```
cfg.nssPin = E22_NSS;
cfg.resetPin = E22_NRST;
cfg.antennaSwitchPin = E22_RXEN;
cfg.busyPin = E22_BUSY;
cfg.dio1Pin = E22_DIO1;
```