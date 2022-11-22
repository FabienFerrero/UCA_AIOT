# 5. GNSS

## Description

Examples in this section demonstrate GPS operations. There are 2 different sketchs: **gnss_quectel_example** for Quectel L96 & **gnss_ublox_example** for Ublox CAM-M8Q.

The sketchs first power on the GNSS module and intialize the control library. These following function is used to power on the GNSS.

```
pinMode(LS_GPS_ENABLE, OUTPUT);
pinMode(LS_GPS_V_BCKP, OUTPUT);
digitalWrite(LS_GPS_ENABLE, HIGH);
digitalWrite(LS_GPS_V_BCKP, HIGH);
```

Because the GNSS Module use the same I2C line as others sensors, it also required to set power on for these sensor.

```
pinMode(SD_ON_OFF, OUTPUT);
digitalWrite(SD_ON_OFF, HIGH);
```

Then every `TX_INTERVAL` second(s), the GNSS information includes Date/Time, Coordinates & Current number fix satellites is extracted, print to Serial Monitor and send to LoRaWAN as well. For LoRaWAN sending, it requires **Device address** (dev_addr), **Network Session Key** (NwkSKey) and **Application Session Key** (AppSKey) to operate. Replace those information in the source code with the ones received from your LoRaWAN Network Server. Make sure that they are all in MSB format.

If you are working with The Things Network, refer to this [Adding Devices](https://www.thethingsindustries.com/docs/devices/adding-devices/) instruction.

```
static uint8_t nwkS_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t appS_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t dev_addr[] = {0x00, 0x00, 0x00, 0x00};
```

The LoRaWAN packet is 17-bytes long and formatted as following:

| 4-bytes    | 4-bytes  | 4-bytes    | 4-bytes  | 1-byte                             |
|------------|----------|------------|----------|------------------------------------|
| EPOCH Time | Latitude | Longtitude | Altitude | Num. of Current fixed satellite(s) |

## The Things Network Packet Formatter

If you are using The Things Network, The Things Stack or The Things Industried, the uplink message can be formated to seperately retrieve **EPOCH Time**, **Latitude**, **Longtitude**, **Altitude**, **Num. of Current fixed satellite(s)**. Otherwise, it only shows the uplink messages in BASE64 format.

To enable this feature, copy the `packet_formatter.js`'s content to application or end-device formatter. Refer to this [Payload Formatters](https://www.thethingsindustries.com/docs/integrations/payload-formatters/) instruction from The Things Industried for further information.

## Credits
- Library for Ublox CAM-M8Q: [sparkfun/SparkFun_Ublox_Arduino_Library](https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library)
- Library for Quectel L96: [stevemarple/MicroNMEA](https://github.com/stevemarple/MicroNMEA)

##### Maintained by Prof. F. Fererro & mtnguyen
