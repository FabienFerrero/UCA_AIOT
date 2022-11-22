#include <Adafruit_SGP30.h>                 // https://github.com/adafruit/Adafruit_SGP30

// SGP30
Adafruit_SGP30 sgp;
int8_t SGP_counter = 0;

void setup(void)
{
  // GPIO
  pinMode(LS_LED_BLUE, OUTPUT);
  pinMode(LS_GPS_ENABLE, OUTPUT);
  pinMode(LS_GPS_V_BCKP, OUTPUT);
  pinMode(SD_ON_OFF, OUTPUT);
  pinMode(LS_VERSION_ENABLE, OUTPUT);

  // Normal running, power ON peripherals
  digitalWrite(LS_GPS_ENABLE, HIGH);
  digitalWrite(LS_GPS_V_BCKP, HIGH);
  digitalWrite(SD_ON_OFF, HIGH);
  digitalWrite(LS_VERSION_ENABLE, HIGH);

  delay(100);

  // Serial
  Serial.begin(115200);
  while (!Serial && (millis() < 5000))
  {
  }
  Serial.println("======= sensor_environment_example =======");
  Serial.println("Initializing");

  analogReadResolution(12);
  Wire.begin();

  delay(200);

  // SGP
  Serial.println("==============================================");
  if (!sgp.begin())
  {
    Serial.println("SGP30: Sensor not found");
  }
  Serial.print("SGP30: Found sensor with SN #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
}

void loop()
{
  Serial.println("**********************************************");
      if (!sgp.IAQmeasure())
      {
        Serial.println("SGP: Measurement failed");
      }
      Serial.print("SGP: TVOC ");
      Serial.print(sgp.TVOC);
      Serial.print(" ppb\t");
      Serial.print("eCO2 ");
      Serial.print(sgp.eCO2);
      Serial.println(" ppm");

      if (!sgp.IAQmeasureRaw())
      {
        Serial.println("SGP: Raw Measurement failed");
      }
      Serial.print("SGP: Raw H2 ");
      Serial.print(sgp.rawH2);
      Serial.print(" \t");
      Serial.print("Raw Ethanol ");
      Serial.println(sgp.rawEthanol);

      SGP_counter++;
      if (SGP_counter >= 30)
      {
        SGP_counter = 0;

        uint16_t TVOC_base, eCO2_base;
        if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
        {
          Serial.println("SGP: Failed to get baseline readings");
        }
        Serial.print("SGP Baseline values: eCO2 0x");
        Serial.print(eCO2_base, HEX);
        Serial.print(" & TVOC 0x");
        Serial.println(TVOC_base, HEX);
      }

  delay(5000);
}
