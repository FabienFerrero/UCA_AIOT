# UCA_AIOT


![alt text](https://github.com/FabienFerrero/DKIOT/blob/main/doc/Untitled.png)


## Installing Board Manager


Please use the official RFTHings repository to configure your Arduino IDE with RFThings DK Blue : https://github.com/RFThings/arduino-STM32L4

## Schematic

The schematic is available in /doc : https://github.com/FabienFerrero/DKIOT/blob/main/doc/LS200-007_SCHEMATIC.pdf

## Supported Library

The board has been successfully tested with several SX1262 library :

* LoRaWAN Mac protocol :
https://github.com/FabienFerrero/basicmac    forked from : https://github.com/LacunaSpace/basicmac

      Warning :add : "#define ARDUINO_STM32L4_LS200" in the code to make it work.

* LoRa Physical layer :
https://github.com/StuartsProjects/SX12XX-LoRa





Warning : SPI PINS for SX1262 must be defined

cfg.nssPin = E22_NSS;
cfg.resetPin = E22_NRST;
cfg.antennaSwitchPin = E22_RXEN;
cfg.busyPin = E22_BUSY;
cfg.dio1Pin = E22_DIO1;
