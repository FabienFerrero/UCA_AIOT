#include <Wire.h>
#include <RTC.h>
#include <time.h>
#include <LibLacuna.h>
#include <MicroNMEA.h>   // http://librarymanager/All#MicroNMEA

// Set a max fix time
#define FIXTIME 180

float gnss_lat;                         // Global node postition lattiude
float gnss_lon;                         // Global node postition longitude
float gnss_alt;                         // Global node postition altitude
float course;                           // Global node course in degree
float speed;                            // Global node speed
uint8_t sat;                            // Global node number of sat

unsigned long startTime, stopTime;
uint8_t GPS_Address = 0x10;

//MicroNMEA library structures
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));


void setup() {
  // put your setup code here, to run once:


  Serial.begin(9600);
  pinMode(LS_GPS_ENABLE, OUTPUT);
  digitalWrite(LS_GPS_ENABLE, LOW); // Off
  delay(2000); // wait for GPS reset

  pinMode(LS_GPS_V_BCKP, OUTPUT);
  digitalWrite(LS_GPS_V_BCKP, HIGH);

  digitalWrite(LS_VERSION_ENABLE, LOW);

  pinMode(LS_INT_MAG, OUTPUT); // make INT_MAG LOW for low-power
  digitalWrite(LS_INT_MAG, LOW);
  Wire.begin();
  
  while (!Serial);
  delay(1000);
  Serial.println("Quectel L96 Test ");
  
  pinMode(LS_LED_BLUE, OUTPUT);
  
  
}

void loop() {

  updategps();

  delay (10000);
}

byte updategps() {

// Switch on GPS
   digitalWrite(LS_GPS_ENABLE, HIGH);
   delay(200);
   Serial.println("Turn ON GPS");   
   startTime = millis();
   long altitude;
   bool alt;

   digitalWrite(LS_LED_BLUE, HIGH);
   delay(200);
   digitalWrite(LS_LED_BLUE, LOW);
   delay(200);
    
   while (millis() - startTime < (FIXTIME*1000))
   {
    
     //While the message isn't complete

   while (!nmea.isValid())
   {
      // Blink while not fixed
      digitalWrite(LS_LED_BLUE, HIGH);
      delay(50);
      digitalWrite(LS_LED_BLUE, LOW);
      delay(100);
      digitalWrite(LS_LED_BLUE, HIGH);
      delay(50);
      digitalWrite(LS_LED_BLUE, LOW);

      Serial.print("GPS time: ");
      Serial.print(nmea.getYear());
      Serial.print("-");
      Serial.print(nmea.getMonth());
      Serial.print("-");
      Serial.print(nmea.getDay());
      Serial.print(" ");
      Serial.print(nmea.getHour());
      Serial.print(":");
      Serial.print(nmea.getMinute());
      Serial.print(":");
      Serial.println(nmea.getSecond());
      
      Serial.print(nmea.getNumSatellites());
      Serial.println(" satellites");
     
  // Get NMEA data from I2C
  Wire.requestFrom(GPS_Address, 255);
  while (Wire.available()) {
    char c = Wire.read();
    nmea.process(c);
    //Serial.print(c);
     }
    delay(2000);
   }


//If a message is recieved print all the informations
   if (nmea.isValid())
   {
      long stopTime = millis();   
      long TTF = stopTime - startTime;  
      // Output GPS information from previous second
      Serial.print("Valid fix in ");
      Serial.print(TTF/1e3);
      Serial.println(" seconds");

      Serial.print("Nav. system: ");
      if (nmea.getNavSystem()== 'P') {Serial.println("Navigation results based only on GPS satellites.");}
      if (nmea.getNavSystem()== 'L') {Serial.println("Navigation results based only on GLONASS satellites.");}
      if (nmea.getNavSystem()== 'A') {Serial.println("Navigation results based only on GAlileo satellites.");}
      if (nmea.getNavSystem()== 'N') {Serial.println("GNSS, navigation results from multiple satellite constellations.");}
      
      else
         Serial.println("none");

      Serial.print("HDOP: ");
      Serial.println(nmea.getHDOP()/10., 1);

      Serial.print("  GPS time: ");
      Serial.print(nmea.getYear());
      Serial.print("-");
      Serial.print(nmea.getMonth());
      Serial.print("-");
      Serial.print(nmea.getDay());
      Serial.print(" ");
      Serial.print(nmea.getHour());
      Serial.print(":");
      Serial.print(nmea.getMinute());
      Serial.print(":");
      Serial.println(nmea.getSecond());
      
      uint32_t unixt = unixTimestamp(nmea.getYear(),nmea.getMonth(),nmea.getDay(),nmea.getHour(),nmea.getMinute(),nmea.getSecond());
      
      Serial.print("  Unix time GPS: ");
      Serial.println(unixt);     

      gnss_lat = nmea.getLatitude()/1.0e6;
      gnss_lon = nmea.getLongitude()/1.0e6;      
      alt = nmea.getAltitude(altitude);
      gnss_alt = altitude/1e3;
      course = nmea.getCourse()/1e3;
      speed = nmea.getSpeed()/1e3;
      sat = nmea.getNumSatellites();

      if (alt) {Serial.print("  3D GPS position: ");}
      else {Serial.print("  2D GPS position: ");}
      Serial.print(gnss_lat,4);
      Serial.print(", ");
      Serial.print(gnss_lon,4);
      Serial.print(" alt: ");
      Serial.print(gnss_alt/1.0e3,2);
      Serial.print(" (");
      Serial.print(sat);
      Serial.println(" satellites)");
      Serial.print("Course : ");
      Serial.print(course);
      Serial.print(" °");
      Serial.print("Speed : ");
      Serial.print(speed);
      Serial.println(" m/s");
      
      
      return(1); //exit function
    }
  }
  
  return(0);  
}


unsigned long unixTimestamp(int year, int month, int day, int hour, int min, int sec) {
  const short days_since_beginning_of_year[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
  int leap_years = ((year-1)-1968)/4
                  - ((year-1)-1900)/100
                  + ((year-1)-1600)/400;
  long days_since_1970 = (year-1970)*365 + leap_years
                      + days_since_beginning_of_year[month-1] + day-1;
  if ( (month>2) && (year%4==0 && (year%100!=0 || year%400==0)) )
    days_since_1970 += 1; /* +leap day, if year is a leap year */
  return sec + 60 * ( min + 60 * (hour + 24*days_since_1970) );
}
