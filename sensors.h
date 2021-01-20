#ifndef _sensors_h_
#define _sensors_h_

/*
 * Sensors
 */
#include "DHTesp.h"
//DH11 on S02
DHTesp dht;

#include "SdsDustSensor.h"
SdsDustSensor sds(Serial);

struct TempAndHumid {
  bool status = false;
  float temperatureC;
  float humidity;
  float heatIndexC;
} tempAndHumid;

struct PMSDS01 {
  bool status = false;
  float pm25;
  float pm10;
} pmData;

bool sensorsEnabled = false;
 
void setup_sensors() {
  // StRt sensors
  dht.setup(config.dht11Pin, DHTesp::DHT11);
  logger.println("DH11 started");

  Serial.begin(9600);
  sds.begin();

  logger.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  logger.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
  logger.println(sds.setCustomWorkingPeriod(config.sdsWorkingPeriodMin).toString()); // sensor sends data every 3 minutes
}

bool read_dust_data(PMSDS01& s) {
  if (!sensorsEnabled) return false;
  
  PmResult pm = sds.readPm();
  if (pm.isOk()) {
    pmData.status = true;
    pmData.pm25 = pm.pm25;
    pmData.pm10 = pm.pm10;
    logger.println(pm.toString());
    return true;
  } else {
    return false;
  }
  
}

bool read_temp_humid(TempAndHumid& s, bool computeHI) {
  if (!sensorsEnabled) return false;
  
  s.status = dht.getStatus();  
  s.humidity = dht.getHumidity();
  s.temperatureC = dht.getTemperature();

  if (computeHI) {
    s.heatIndexC = dht.computeHeatIndex(s.temperatureC, s.humidity, false);
  } else {
    s.heatIndexC = 0;
  }

#ifdef VERBOSE_SERIAL
  logger.print(dht.getStatusString());
  logger.print("\t");
  logger.print(s.humidity, 1);
  logger.print("\t\t");
  logger.print(s.temperatureC, 1);
  logger.print("\t\t");
  logger.print(dht.toFahrenheit(s.temperatureC), 1);
  logger.print("\t\t");
  logger.print(dht.computeHeatIndex(s.temperatureC, s.humidity, false), 1);
  logger.print("\t\t");
  logger.println(dht.computeHeatIndex(dht.toFahrenheit(s.temperatureC), s.humidity, true), 1);
#endif
  return true;
}

#endif
