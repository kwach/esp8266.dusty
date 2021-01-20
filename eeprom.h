#ifndef _eeprom_h_
#define _eeprom_h_
/*
 * Settings 
 */

#include <EEPROM.h>
#include <Ethernet.h>

//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define WIFI_PASS_MAX_LEN 64
#define WIFI_SSID_MAX_LEN 32

struct Config {
  char version[6] = "0.1";
  char ssid[WIFI_SSID_MAX_LEN];
  char password[WIFI_PASS_MAX_LEN];

  int dht11Pin = 2;
  int sdsWorkingPeriodMin = 3;

  bool enable_dhcp = true;
  IPAddress ip = IPAddress(192, 168, 4, 1);
  IPAddress gw = IPAddress(192, 168, 4, 1);
  IPAddress mask = IPAddress(255, 255, 255, 0);
  IPAddress dns1 = IPAddress(1, 1, 1, 1);
  IPAddress dns2 = IPAddress(2, 2, 2, 2);
} config;

void setup_config() { 
  logger.println("Configuring EEPROM...");
  EEPROM.begin(512);
}

/* address in eeprom 
 *  ssid - 32 bytes
 *  pass - 
 */
bool read_config() {
  logger.println("Reading config from EEPROM...");
  Config c;
  EEPROM.get(0, c);

  if (c.version == "0.1") {
    memcpy(&config, &c, sizeof(Config));
    logger.print("version: "); logger.println(config.version);
    logger.println("ssid: "); logger.println(config.ssid);
    logger.println("password:"); logger.println(config.ssid);
    logger.println("dh11 pin:"); logger.println(config.dht11Pin);
    logger.println("sds working period: "); logger.println(config.sdsWorkingPeriodMin);
    return true;
  } else {
    logger.println("No config in memory");    
    return false;
  }
}

void store_config() {
  logger.println("Storing config to EEPROM...");  
  EEPROM.put(0, config);
  EEPROM.end();
}

#endif //_eeprom_h_
