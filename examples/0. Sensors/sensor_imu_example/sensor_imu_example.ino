#include <ICM_20948.h> // https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary

// ICM
ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object

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
  Serial.println("======= sensor_imu_example =======");
  Serial.println("Initializing");

  Wire.begin();

  delay(200);

  // ICM
  Serial.println("==============================================");
  bool ICM_initialized = false;
  for (int i = 1; i <= 5; i++)
  {
    myICM.begin(Wire, 0);
    Serial.print("ICM: ");
    Serial.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      Serial.println("ICM: Try again....");
      Serial.print(i);
    }
    else
    {
      Serial.println("ICM: Successful");
      ICM_initialized = true;
    }
  }
  if (!ICM_initialized)
  {
    Serial.print("ICM: Failed");
  }
}

void loop()
{
  Serial.println("**********************************************");
  if (myICM.dataReady())
  {
    myICM.getAGMT(); // The values are only updated when you call 'getAGMT'
    // printRawAGMT( myICM.agmt );     // Uncomment this to see the raw values, taken directly from the agmt structure
    ICM_printScaledAGMT(&myICM); // This function takes into account the scale settings from when the measurement was made to calculate the values with units
  }
  else
  {
    Serial.println("ICM: Waiting for data");
  }

  delay(100);
}

void ICM_printScaledAGMT(ICM_20948_I2C *sensor)
{
  Serial.print("ICM: Scaled. Acc (mg) [ ");
  printFormattedFloat(sensor->accX(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->accY(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->accZ(), 5, 2);
  Serial.print(" ], Gyr (DPS) [ ");
  printFormattedFloat(sensor->gyrX(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->gyrY(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->gyrZ(), 5, 2);
  Serial.print(" ], Mag (uT) [ ");
  printFormattedFloat(sensor->magX(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->magY(), 5, 2);
  Serial.print(", ");
  printFormattedFloat(sensor->magZ(), 5, 2);
  Serial.print(" ], Tmp (C) [ ");
  printFormattedFloat(sensor->temp(), 5, 2);
  Serial.print(" ]");
  Serial.println();
}

void printFormattedFloat(float val, uint8_t leading, uint8_t decimals)
{
  float aval = abs(val);
  if (val < 0)
  {
    Serial.print("-");
  }
  else
  {
    Serial.print(" ");
  }
  for (uint8_t indi = 0; indi < leading; indi++)
  {
    uint32_t tenpow = 0;
    if (indi < (leading - 1))
    {
      tenpow = 1;
    }
    for (uint8_t c = 0; c < (leading - 1 - indi); c++)
    {
      tenpow *= 10;
    }
    if (aval < tenpow)
    {
      Serial.print("0");
    }
    else
    {
      break;
    }
  }
  if (val < 0)
  {
    Serial.print(-val, decimals);
  }
  else
  {
    Serial.print(val, decimals);
  }
}
