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

/* Support REGION
    RFT_REGION_EU863_870
    RFT_REGION_US902_928
    RFT_REGION_CN470_510
    RFT_REGION_AU915_928
    RFT_REGION_AS920_923
    RFT_REGION_AS923_925
    RFT_REGION_KR920_923
    RFT_REGION_IN865_867
*/

#include <RFThings.h>
#include <rfthings_sx126x.h>
#include <RTC.h>
#include <time.h>
#include <Wire.h>
#include <Sgp4.h>      // https://github.com/Hopperpop/Sgp4-Library
#include <MicroNMEA.h> // http://librarymanager/All#MicroNMEA
#include <ICM_20948.h> // https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary

/**************** PROJECT CONFIGURATION ****************/
/**
 * Modify this section to adapt to your scenario.
 */

// #define DEBUG_SLEEP
#define SAT_PACKET_PERIOD_S (15)                     // 15 seconds
#define MIN_PASS_ELAVATION (25)                      // 25 degrees
#define TERRESTRIAL_STATUS_PACKET_PERIOD_S (15 * 60) // 15 minutes
#define GNSS_UPDATE_PERIOD_S (12 * 60 * 60)          // 12 hours
#define GNSS_RESCHEDULE_OFFSET_S (30 * 60)           // 30 minutes
// #define GNSS_FAKE_COORDINATES

// Update the latest TLE from here https://www.n2yo.com/satellite/?s=47948 or Space-Track.com
// Below TLE is generated at Oct 10, 2023 12:00 GMT
char satname[] = "LACUNASAT-2B";
char tle_line1[] = "1 47948U 21022S   23282.68847272  .00070269  00000-0  22799-2 0  9995"; // Line one from the TLE data
char tle_line2[] = "2 47948  97.4915 184.4965 0007348 278.5094  81.5314 15.31673100140591"; // Line two from the TLE data

// Terrestrial device: elliot-toulouse-nov22-24/irt-lacuna-2-terrestrial
static uint8_t terrestrial_app_eui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t terrestrial_app_key[] = {0x3A, 0x5E, 0xAF, 0x15, 0x84, 0x64, 0xB0, 0x21, 0xFD, 0xEA, 0xB4, 0x9C, 0xEE, 0xFA, 0x13, 0x1C};
static uint8_t terrestrial_dev_eui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x1A, 0xA8};
uint8_t *terrestrial_dev_addr;
uint8_t *terrestrial_nwkS_key;
uint8_t *terrestrial_appS_key;

// Satellite device: elliot-toulouse-nov22-24/irt-lacuna-2
static uint8_t sat_nwkS_key[] = {0x23, 0xB3, 0xCF, 0xBD, 0x05, 0x17, 0xAB, 0x63, 0x25, 0xB9, 0x7A, 0xD0, 0x67, 0x3F, 0xD5, 0xA4};
static uint8_t sat_appS_key[] = {0x5F, 0xA2, 0xF7, 0xDC, 0x23, 0x7C, 0x5A, 0x97, 0x30, 0xC7, 0x1B, 0x37, 0x67, 0xCA, 0x03, 0xB0};
static uint8_t sat_dev_addr[] = {0x26, 0x0B, 0x0B, 0x0C};

/*******************************************************/

// Macro define
#if defined(DEBUG_SLEEP)
#define LOG_D(...) Serial.print(__VA_ARGS__)
#define LOG_D_NL(...) Serial.println(__VA_ARGS__)
#else
#define LOG_D(...)
#define LOG_D_NL(...)
#endif

// LoRa Module
rfthings_sx126x sx1262_satellite(E22_NSS, E22_NRST, E22_BUSY, E22_DIO1, E22_RXEN);
rfthings_sx126x sx1262_terrestrial(E22_NSS, E22_NRST, E22_BUSY, E22_DIO1, E22_RXEN);
rft_status_t status;

// LoRaWAN & LoRaSpace payload
char payload[255];
uint32_t payload_len;

// GNSS
#define I2C_GPS_QUECTEL_ADDRESS 0X10
#define WAITING_FOR_STABLIZING_SEC 30 // Wait for 30 seconds after 3D fix detect
uint32_t gnss_unix_time = 1678974432;
int32_t gnss_latitude = 436149513;
int32_t gnss_longitude = 70713642;
int32_t gnss_altitude = 20000;
uint8_t gnss_siv = 0;

// Event timestamp
uint32_t next_gnss_update = 0;
uint32_t last_gnss_fix_time = 0;

uint32_t next_satellite_pass_start = 0;
uint32_t next_satellite_pass_stop = 0;

uint32_t next_status_packet = 0;

// Satellite predictor
Sgp4 predictor;

// ICM
ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object

void setup(void)
{
    analogReadResolution(12);

#if defined(DEBUG_SLEEP)
    Serial.begin(115200);
    while (!Serial && (millis() < 5000))
    {
    }
#endif

    LOG_D("Demo: Space communication demonstration with Lacuna Space\n");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);

    // GNSS Init
    gnss_init();

    // LoRa Init
    lora_init();

    // Sensors
    sensor_init();

    gnss_get_time_and_coordinates();
    predict_next_sat_pass();

    send_terrestrial_status_packet();

    // Done initialization, go to sleep until the next event
    digitalWrite(LED_BUILTIN, HIGH);
    delay(125);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(125);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(125);
    digitalWrite(LED_BUILTIN, LOW);

    uint32_t next_event_epoch = find_next_event();
    LOG_D("Finish initialization\n");
    LOG_D("Set sleep, next wakeup epoch: ");
    LOG_D_NL(next_event_epoch);
    set_board_sleep(next_event_epoch);
    LOG_D_NL("Board wakeup");
}

void loop(void)
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(125);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(125);
    digitalWrite(LED_BUILTIN, LOW);

    uint32_t epoch_now = RTC.getEpoch();
    LOG_D("Current epoch: ");
    LOG_D_NL(epoch_now);

    // Check Satelite pass
    if ((epoch_now >= next_satellite_pass_start) && (epoch_now < next_satellite_pass_stop))
    {
        LOG_D_NL("Wake up reason: Satellite pass");

        send_satellite_packet();
        predict_next_sat_pass();
    }

    // Check Status sending time
    if (epoch_now >= next_status_packet)
    {
        LOG_D_NL("Wake up reason: Send status packet");

        send_terrestrial_status_packet();
    }

    // Check GNSS update time
    if (epoch_now >= next_gnss_update)
    {
        LOG_D_NL("Wake up reason: Update GNSS time and coordinates");

        gnss_get_time_and_coordinates();
        correct_gnss_update_time();
    }

    // Go back to sleep
    uint32_t next_event_epoch = find_next_event();
    LOG_D("Set sleep, next wakeup epoch: ");
    LOG_D_NL(next_event_epoch);
    set_board_sleep(next_event_epoch);
    LOG_D_NL("Board wakeup");
}

void correct_gnss_update_time(void)
{
    // If next GNSS update time violate the next satellite pass guard time, change it after the pass
    if ((next_gnss_update >= (next_satellite_pass_start - GNSS_RESCHEDULE_OFFSET_S)) && (next_gnss_update < (next_satellite_pass_stop + GNSS_RESCHEDULE_OFFSET_S)))
    {
        next_gnss_update = next_satellite_pass_stop + GNSS_RESCHEDULE_OFFSET_S;

        LOG_D("Detect a close time between next satellite pass and GNSS update time, reschedule GNSS Update time to: ");
        LOG_D_NL(next_gnss_update);
    }
}

void gnss_init(void)
{
    Wire.begin();

    // Power on GPS module and I2C line
    pinMode(LS_GPS_ENABLE, OUTPUT);
    digitalWrite(LS_GPS_ENABLE, HIGH);
    pinMode(LS_GPS_V_BCKP, OUTPUT);
    digitalWrite(LS_GPS_V_BCKP, HIGH);

    digitalWrite(LS_VERSION_ENABLE, LOW);

    pinMode(SD_ON_OFF, OUTPUT);
    digitalWrite(SD_ON_OFF, HIGH);

    delay(500);

    Wire.beginTransmission(I2C_GPS_QUECTEL_ADDRESS);
    byte error = Wire.endTransmission();

    if (error == 0)
    {
        LOG_D_NL("GNSS Initialization: OK");
    }
    else
    {
        digitalWrite(LED_BUILTIN, HIGH);
        LOG_D("GNSS Initialization: Fail");
        while (1)
        {
            LOG_D_NL(F("Quectel GPS not detected at default I2C address. Please check wiring."));
            delay(1000);
        }
    }
}

void lora_init(void)
{
    /* LoRaWAN Satellite */
    status = sx1262_satellite.init(RFT_REGION_EU863_870);
    LOG_D("SX126x Satellite Initialization: ");
    LOG_D_NL(rft_status_to_str(status));
    if (status != RFT_STATUS_OK)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        while (1)
        {
            delay(1000);
            LOG_D("SX126x Satellite Initialization: ");
            LOG_D_NL(rft_status_to_str(status));
        }
    }

    // LoRaWAN parameters
    sx1262_satellite.set_lorawan_activation_type(RFT_LORAWAN_ACTIVATION_TYPE_ABP);
    sx1262_satellite.set_application_session_key(sat_appS_key);
    sx1262_satellite.set_network_session_key(sat_nwkS_key);
    sx1262_satellite.set_device_address(sat_dev_addr);

    // Config LR-FHSS parameters
    sx1262_satellite.set_lrfhss_codingRate(RFT_LRFHSS_CODING_RATE_1_3);
    sx1262_satellite.set_lrfhss_bandwidth(RFT_LRFHSS_BANDWIDTH_335_9_KHZ);
    sx1262_satellite.set_lrfhss_grid(RFT_LRFHSS_GRID_3_9_KHZ);
    sx1262_satellite.set_lrfhss_hopping(true);
    sx1262_satellite.set_lrfhss_nbSync(4);
    sx1262_satellite.set_lrfhss_frequency(862750000);
    sx1262_satellite.set_lrfhss_power(21);

    /* LoRaWAN Terrestrial */
    status = sx1262_terrestrial.init(RFT_REGION_EU863_870);
    LOG_D("SX126x Terrestrial Initialization: ");
    LOG_D_NL(rft_status_to_str(status));
    if (status != RFT_STATUS_OK)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        while (1)
        {
            delay(1000);
            LOG_D("SX126x Terrestrial Initialization: ");
            LOG_D_NL(rft_status_to_str(status));
        }
    }

    // LoRaWAN parameters
    sx1262_terrestrial.set_lorawan_activation_type(RFT_LORAWAN_ACTIVATION_TYPE_OTAA);
    sx1262_terrestrial.set_devive_eui(terrestrial_dev_eui);
    sx1262_terrestrial.set_application_eui(terrestrial_app_eui);
    sx1262_terrestrial.set_application_key(terrestrial_app_key);
    sx1262_terrestrial.set_rx1_delay(5000);

    bool join_accepted = false;
    while (!join_accepted) // OTAA Join request loop
    {
        LOG_D("Sending join request: ");
        status = sx1262_terrestrial.send_join_request(NULL, NULL);

        if (status == RFT_STATUS_OK)
        {
            terrestrial_dev_addr = sx1262_terrestrial.get_device_address();
            terrestrial_nwkS_key = sx1262_terrestrial.get_network_session_key();
            terrestrial_appS_key = sx1262_terrestrial.get_application_session_key();

            LOG_D_NL("Join accept!");
            join_accepted = true;

            LOG_D("Device address: ");
            for (int i = 0; i < 4; i++)
            {
                LOG_D(otaa_dev_addr[i], HEX);
                LOG_D(" ");
            }
            LOG_D_NL();

            LOG_D("Network session key: ");
            for (int i = 0; i < 16; i++)
            {
                LOG_D(otaa_nwkS_key[i], HEX);
                LOG_D(" ");
            }
            LOG_D_NL();

            LOG_D("Application session address: ");
            for (int i = 0; i < 16; i++)
            {
                LOG_D(otaa_appS_key[i], HEX);
                LOG_D(" ");
            }
            LOG_D_NL();
        }
        else
        {
            LOG_D_NL(rft_status_to_str(status));
        }

        delay(5000);
    }
}

void sensor_init(void)
{
    digitalWrite(LS_GPS_ENABLE, HIGH);
    digitalWrite(LS_GPS_V_BCKP, HIGH);
    digitalWrite(SD_ON_OFF, HIGH);

    Wire.begin();

    delay(1000);

    // ICM
    myICM.begin(Wire, 0);
    LOG_D("ICM: ");
    LOG_D_NL(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
        LOG_D_NL("ICM: Try again....");
    }
    else
    {
        LOG_D_NL("ICM: Successful");
    }

    digitalWrite(LS_GPS_ENABLE, LOW);
    digitalWrite(LS_GPS_V_BCKP, HIGH);
    digitalWrite(SD_ON_OFF, LOW);
}

void gnss_get_time_and_coordinates(void)
{
#if !defined(GNSS_FAKE_COORDINATES)
    bool flagRead = false;
    char revChar = 0;
    String tmpBuff = "";
    const String str3D = "$GNGSA,A,3";

    // GPS Quectel
    char nmeaBuffer[255];
    MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

    byte fixType = 0;
    uint32_t fix_3d_detect_epoch = 0;

    LOG_D_NL("Getting GNSS Time & Coordinates");

    // Power on GPS module and I2C line
    digitalWrite(LS_GPS_ENABLE, HIGH);
    digitalWrite(LS_GPS_V_BCKP, HIGH);
    digitalWrite(SD_ON_OFF, HIGH);

    delay(1000);

    nmea.clear();
    uint32_t fix_time_start = RTC.getEpoch();
    delay(200);

    while (!((fixType == 3) && ((RTC.getEpoch() - fix_3d_detect_epoch) > WAITING_FOR_STABLIZING_SEC))) // Wait for 3D Fix & t > WAITING_FOR_STABLIZING_SEC only
    {
        Wire.requestFrom(I2C_GPS_QUECTEL_ADDRESS, 255);
        while (Wire.available())
        {
            revChar = Wire.read();
            nmea.process(revChar);

            if (revChar == '$')
            {
                tmpBuff = "";
                flagRead = true;
            }

            if (flagRead)
            {
                tmpBuff += revChar;
                if (tmpBuff.equals(str3D))
                {
                    // LOG_D_NL("Detected 3D Fix");

                    if ((fixType == 0) && nmea.isValid())
                    {
                        fixType = 3;
                        fix_3d_detect_epoch = RTC.getEpoch();
                    }
                }

                if (revChar == '\n')
                {
                    flagRead = false;
                    LOG_D_NL(tmpBuff);
                }
            }
        }

        delay(100);
    }

    last_gnss_fix_time = RTC.getEpoch() - fix_time_start;

    gnss_unix_time = unixTimestamp(nmea.getYear(), nmea.getMonth(), nmea.getDay(), nmea.getHour(), nmea.getMinute(), nmea.getSecond());
    gnss_latitude = nmea.getLatitude() * 10;
    gnss_longitude = nmea.getLongitude() * 10;
    long alt_tmp = 0;
    if (nmea.getAltitude(alt_tmp))
    {
        gnss_altitude = alt_tmp;
    }
    gnss_siv = nmea.getNumSatellites();

    LOG_D_NL("Update new GNSS Time & Coordinates");
    LOG_D("Fix time (seconds): ");
    LOG_D_NL(last_gnss_fix_time);
    LOG_D("Unix time: ");
    LOG_D_NL(gnss_unix_time);

    LOG_D("Lat: ");
    LOG_D(gnss_latitude);
    LOG_D_NL(" 10^-7 deg");
    LOG_D("Lon: ");
    LOG_D(gnss_longitude);
    LOG_D_NL(" 10^-7 deg");
    LOG_D("Alt: ");
    LOG_D(gnss_altitude);
    LOG_D_NL(" millimeters");

    // Power off GPS module and I2C line
    digitalWrite(LS_GPS_ENABLE, LOW);
    digitalWrite(LS_GPS_V_BCKP, HIGH);
    digitalWrite(SD_ON_OFF, LOW);
#endif

    // Update time on STM32 RTC Module
    RTC.setEpoch(gnss_unix_time);
    next_gnss_update = RTC.getEpoch() + GNSS_UPDATE_PERIOD_S;
}

void predict_next_sat_pass(void)
{
    double lat = (double)(gnss_latitude / 1.0e7);
    double lon = (double)(gnss_longitude / 1.0e7);
    double alt = (double)(gnss_altitude / 1.0e3);
    predictor.site(lat, lon, alt);

    predictor.init(satname, tle_line1, tle_line2);

    passinfo overpass;                            // structure to store overpass info
    predictor.initpredpoint(RTC.getEpoch(), 0.0); // finds the startpoint

    bool good_pass_found = false;
    bool predict_result;
    for (uint8_t i = 0; i < 15; i++) // Search for the next 15 pass for a good max elavtion
    {
        predict_result = predictor.nextpass(&overpass, 20); // search for the next overpass, if there are more than 20 maximums below the horizon it returns false
        if (predict_result && (overpass.maxelevation > MIN_PASS_ELAVATION))
        {
            good_pass_found = true;
            break; // Stop the prediction iteration
        }
        else
        {
        }
    }

    LOG_D_NL("Predict next satellite pass");
    if (good_pass_found)
    {
        next_satellite_pass_start = getUnixFromJulian(overpass.jdstart) - (5 * 60); // Add 5 minute margin
        next_satellite_pass_stop = getUnixFromJulian(overpass.jdstop) + (5 * 60);   // Add 5 minute margin

        LOG_D_NL("A good pass found");
        LOG_D("Start: ");
        LOG_D(next_satellite_pass_start);
        LOG_D(" | Stop: ");
        LOG_D(next_satellite_pass_stop);
        LOG_D(" | Max. Elavation: ");
        LOG_D_NL(overpass.maxelevation);

        correct_gnss_update_time();
    }
    else
    {
        next_satellite_pass_start = 0;
        next_satellite_pass_stop = 0;

        LOG_D_NL("Pass NOT found");
    }
}

unsigned long unixTimestamp(int year, int month, int day, int hour, int min, int sec)
{
    const short days_since_beginning_of_year[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int leap_years = ((year - 1) - 1968) / 4 - ((year - 1) - 1900) / 100 + ((year - 1) - 1600) / 400;
    long days_since_1970 = (year - 1970) * 365 + leap_years + days_since_beginning_of_year[month - 1] + day - 1;
    if ((month > 2) && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
        days_since_1970 += 1; /* +leap day, if year is a leap year */
    return sec + 60 * (min + 60 * (hour + 24 * days_since_1970));
}

void generate_packet(bool send_to_satellite)
{
    LOG_D("generate_packet(");
    LOG_D(send_to_satellite);
    LOG_D_NL(")");

    digitalWrite(LS_GPS_ENABLE, HIGH);
    digitalWrite(LS_GPS_V_BCKP, HIGH);
    digitalWrite(SD_ON_OFF, HIGH);

    // Clear payload data
    memset((void *)payload, 0, 255);
    payload_len = 0;

    delay(200);

    myICM.startupDefault(false);

#if 0
    // Packet type: '1' = Send to satellite | '0' = Send to terrestrial gateways
    payload[0] = (send_to_satellite ? 1 : 0);

    // RTC Epoch at packet build time
    uint32_t now_epoch = RTC.getEpoch();
    payload[1] = (now_epoch >> 0) & 0xff;
    payload[2] = (now_epoch >> 8) & 0xff;
    payload[3] = (now_epoch >> 16) & 0xff;
    payload[4] = (now_epoch >> 24) & 0xff;

    // Baterry level in ADC value
    uint16_t voltage_adc = (uint16_t)analogRead(LS_BATVOLT_PIN);
    payload[5] = (voltage_adc >> 0) & 0xff;
    payload[6] = (voltage_adc >> 8) & 0xff;

    // TLE Age
    double jdC = predictor.satrec.jdsatepoch;
    uint32_t tle_age_unix = now_epoch - getUnixFromJulian(jdC);
    payload[7] = (tle_age_unix >> 0) & 0xff;
    payload[8] = (tle_age_unix >> 8) & 0xff;
    payload[9] = (tle_age_unix >> 16) & 0xff;

    // Next satellite pass (terrestrial packet only)
    if ((next_satellite_pass_start != 0) && (next_satellite_pass_start > now_epoch))
    {
        uint32_t time_to_next_pass = next_satellite_pass_start - now_epoch;

        // 3 byte give maximum 194 days offset to epoch
        payload[10] = (time_to_next_pass >> 0) & 0xff;
        payload[11] = (time_to_next_pass >> 8) & 0xff;
        payload[12] = (time_to_next_pass >> 16) & 0xff;
    }

    // Next GNSS update time
    if ((next_gnss_update != 0) && (next_gnss_update > now_epoch))
    {
        uint32_t time_to_next_gnss_update = next_gnss_update - now_epoch;

        // 2 byte give maximum 18 hours offset to epoch
        payload[13] = (time_to_next_gnss_update >> 0) & 0xff;
        payload[14] = (time_to_next_gnss_update >> 8) & 0xff;
    }

    // Last GNSS fix time
    payload[15] = (last_gnss_fix_time >> 0) & 0xff;
    payload[16] = (last_gnss_fix_time >> 8) & 0xff;

    // Latitude
    payload[17] = (gnss_latitude >> 0) & 0xff;
    payload[18] = (gnss_latitude >> 8) & 0xff;
    payload[19] = (gnss_latitude >> 16) & 0xff;
    payload[20] = (gnss_latitude >> 24) & 0xff;

    // Longtitude
    payload[21] = (gnss_longitude >> 0) & 0xff;
    payload[22] = (gnss_longitude >> 8) & 0xff;
    payload[23] = (gnss_longitude >> 16) & 0xff;
    payload[24] = (gnss_longitude >> 24) & 0xff;

    payload_len = 25;

#else

    // Packet type: '1' = Send to satellite | '0' = Send to terrestrial gateways
    payload[0] = (send_to_satellite ? 1 : 0);

    // Temperature
    uint32_t ICM_read_timeout_start = millis();
    while ((!myICM.dataReady()) && (millis() - ICM_read_timeout_start < 5000))
    {
    }
    if (myICM.dataReady())
    {
        myICM.getAGMT();

        payload[1] = (int(myICM.temp() * 100) >> 0) & 0xff; // Temperature in Celsius 2 bytes
        payload[2] = (int(myICM.temp() * 100) >> 8) & 0xff;
        LOG_D("Temperature: ");
        LOG_D_NL(myICM.temp());
    }
    else
    {
        LOG_D_NL("Accelerometer not available");
    }

    // GNSS
    payload[3] = (gnss_latitude >> 0) & 0xff;
    payload[4] = (gnss_latitude >> 8) & 0xff;
    payload[5] = (gnss_latitude >> 16) & 0xff;
    payload[6] = (gnss_latitude >> 24) & 0xff;

    payload[7] = (gnss_longitude >> 0) & 0xff;
    payload[8] = (gnss_longitude >> 8) & 0xff;
    payload[9] = (gnss_longitude >> 16) & 0xff;
    payload[10] = (gnss_longitude >> 24) & 0xff;

    payload[11] = (gnss_altitude >> 0) & 0xff;
    payload[12] = (gnss_altitude >> 8) & 0xff;
    payload[13] = (gnss_altitude >> 16) & 0xff;
    payload[14] = (gnss_altitude >> 24) & 0xff;

    // Accelerometer
    ICM_read_timeout_start = millis();
    while ((!myICM.dataReady()) && (millis() - ICM_read_timeout_start < 5000))
    {
    }
    if (myICM.dataReady())
    {
        myICM.getAGMT();
        float accX = myICM.accX();
        float accY = myICM.accY();
        float accZ = myICM.accZ();

        LOG_D("Accelerometer: ");
        LOG_D(accX);
        LOG_D(" ");
        LOG_D(accY);
        LOG_D(" ");
        LOG_D_NL(accZ);

        payload[15] = (int)((accX / 2000.0) * 127); // -127..127
        payload[16] = (int)((accY / 2000.0) * 127); // -127..127
        payload[17] = (int)((accZ / 2000.0) * 127); // -127..127
    }
    else
    {
        LOG_D_NL("Accelerometer not available");
    }

    // Battery
    uint16_t voltage_adc = (uint16_t)analogRead(LS_BATVOLT_PIN);
    payload[18] = (voltage_adc >> 0) & 0xff;
    payload[19] = (voltage_adc >> 8) & 0xff;
    LOG_D("Battery: ");
    LOG_D_NL(voltage_adc);

    // Num of satellites
    payload[20] = gnss_siv & 0xff;

    // TLE Age
    double jdC = predictor.satrec.jdsatepoch;
    uint32_t now_epoch = RTC.getEpoch();
    uint32_t tle_age_unix = getUnixFromJulian(jdC);
    uint32_t tle_age_days = (now_epoch - tle_age_unix) / (24 * 60 * 60);
    if (tle_age_days > 255)
    {
        tle_age_days = 255;
    }
    payload[21] = tle_age_days & 0xff;
    LOG_D("TLE Age: ");
    LOG_D(jdC);
    LOG_D(" ");
    LOG_D(now_epoch);
    LOG_D(" ");
    LOG_D(tle_age_unix);
    LOG_D(" ");
    LOG_D_NL(tle_age_days);

    // Last GNSS fix time
    payload[22] = (last_gnss_fix_time < 255) ? (last_gnss_fix_time & 0xff) : 255;

    payload_len = 23;

#endif

    digitalWrite(LS_GPS_ENABLE, LOW);
    digitalWrite(LS_GPS_V_BCKP, HIGH);
    digitalWrite(SD_ON_OFF, LOW);
}

void send_terrestrial_status_packet(void)
{
    generate_packet(false);
    LOG_D("Sending LoRaWAN terrestrial message: ");
    status = sx1262_terrestrial.send_uplink((byte *)payload, payload_len, NULL, NULL);
    LOG_D_NL(rft_status_to_str(status));

    delay(1000);

    next_status_packet = RTC.getEpoch() + TERRESTRIAL_STATUS_PACKET_PERIOD_S;
}

void send_satellite_packet(void)
{
    // Send packet
    uint32_t epoch_now = RTC.getEpoch();
    while ((epoch_now >= next_satellite_pass_start) && (epoch_now < next_satellite_pass_stop))
    {
        generate_packet(true);
        LOG_D("Sending LR-FHSS message: ");
        status = sx1262_satellite.send_lorawan_over_lrfhss((byte *)payload, payload_len);
        LOG_D_NL(rft_status_to_str(status));

        delay(1000);

        set_board_sleep(RTC.getEpoch() + SAT_PACKET_PERIOD_S);
        epoch_now = RTC.getEpoch();
    }
}

void rtcAlarmMatch() {}

uint32_t find_next_event(void)
{
    uint32_t next_event_epoch = 0xffffffff;

    if ((next_event_epoch > next_satellite_pass_start) && (next_satellite_pass_start != 0))
    {
        next_event_epoch = next_satellite_pass_start;
    }

    if ((next_event_epoch > next_gnss_update) && (next_gnss_update != 0))
    {
        next_event_epoch = next_gnss_update;
    }

    if ((next_event_epoch > next_status_packet) && (next_status_packet != 0))
    {
        next_event_epoch = next_status_packet;
    }

    return next_event_epoch;
}

void set_board_sleep(uint32_t wakeup_epoch)
{
#if defined(DEBUG_SLEEP)
    uint32_t delay_duration = wakeup_epoch - RTC.getEpoch();
    delay(delay_duration * 1000);
#else
    uint32_t now_epoch = RTC.getEpoch();
    if (now_epoch >= wakeup_epoch)
    {
        return;
    }
    else if (wakeup_epoch - now_epoch <= 5)
    {
        delay((wakeup_epoch - now_epoch) * 1000);
    }
    else
    {
        time_t t;
        struct tm tm;

        t = (time_t)wakeup_epoch;
        gmtime_r(&t, &tm);

        RTC.setAlarmTime(tm.tm_hour, tm.tm_min, tm.tm_sec);
        RTC.setAlarmDay(tm.tm_mday);

        RTC.enableAlarm(RTC.MATCH_HHMMSS);
        RTC.attachInterrupt(rtcAlarmMatch);

        digitalWrite(LS_GPS_ENABLE, LOW);
        digitalWrite(LS_VERSION_ENABLE, LOW);
        digitalWrite(LS_GPS_V_BCKP, HIGH);
        digitalWrite(SD_ON_OFF, LOW);

        SPI.end();
        delay(10);
        STM32.stop();

        SPI.begin();
    }
#endif
}
