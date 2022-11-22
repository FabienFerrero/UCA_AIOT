# 0. Sensors

## Description

The examples included in this section are dedicated to test integrated sensors on the UCA AIoT Board. Each sketch demonstrates the library initialization & a simple data read application for a sensors.

- [**sensor_environment_example**](./sensor_environment_example/) for the Environmental sensor Sensirion SGP30-2.5K
- [**sensor_imu_example**](./sensor_imu_example/) IMUs TDK Invensense ICM-20948

# Board Hardware Test
Except for the [**uca_aiot_sensors_test_example**](./uca_aiot_sensors_test_example/) sketch, this is the board hardware testing sketch. It includes the test of following components in one sketch:

- Ambient Light Sensor: LITEON LTR-303ALS-01
- Altimetric Presure sensor: HOPERF HP203B
- Environmental sensor: Sensirion SGP30-2.5K
- 9-DoF IMUs: TDK Invensense ICM-20948
- GNSS Module (Quectel L96 or Ublox CAM-M8Q)
- MicroSD Card slot

All components are checked in term of I2C connectivity, controlling library initialization, data reading. Results of the test will be printed in Serial Monitor of the Board.

This sketch is also support Firmware Uploading for GNSS Module Quectel L96. If you need to modify the firmware, hold **USR_BTN** at board reset. Firmware Uploading mode running is indicated by the **LED_BUILTIN** (LED Blue) is always ON. For further information on this mode, please contacts RFThings Support Team at [support@rfthings.com.vn](mailto:support@rfthings.com.vn).


##### Maintained by Prof. F. Fererro & mtnguyen
