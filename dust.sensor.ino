/*
  Instructions:
  - Point your browser to http://esp8266.local, you should see a response.

*/

#if !( defined(ESP8266) || defined(ESP32) )
#error This code is intended to run only on the ESP8266 and ESP32 boards ! Please check your Tools->Board setting.
#endif

// this will enable serial in loop()
// disconnect SDS01 for this to work

struct Log {
  bool enabled = true;

  void begin(int boud) {
    Serial.begin(boud);
  }

  template <typename T>
  void println(const T& s) {
    if (enabled) {
      Serial.println(s);
    }
  }

  template <typename T>
  void print(const T& s) {
    if (enabled) {
      Serial.print(s);
    }
  }
} logger;



#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <PageBuilder.h> // for some reason it won't work included in http.ino

// HTTP server at port 80 will respond to HTTP requests
ESP8266WebServer server(80); //Server on port 80

#include <ESP8266mDNS.h> // http://esp8266.local - linux
#include <ESP8266NetBIOS.h> // http://esp8266 - windows

#define DRD_TIMEOUT             10
#define DRD_ADDRESS             2
#include <ESP_DoubleResetDetector.h>            //https://github.com/khoih-prog/ESP_DoubleResetDetector
DoubleResetDetector* drd;

//TODO: move fs to fs.h
// Select the FileSystem by uncommenting one of the lines below
//#define USE_SPIFFS
#define USE_LITTLEFS
//#define USE_SDFS

// File resources
#if defined USE_SPIFFS
#include <FS.h>
FS* fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#elif defined USE_LITTLEFS
#include <LittleFS.h>
FS* fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();
#elif defined USE_SDFS
#include <SDFS.h>
FS* fileSystem = &SDFS;
SDFSConfig fileSystemConfig = SDFSConfig();
// fileSystemConfig.setCSPin(chipSelectPin);
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif

#include "eeprom.h"
#include "captive.h"
#include "wifi.h"
#include "sensors.h"
#include "http.h"

void setup_fs() {
  // FILESYSTEM INIT
  fileSystemConfig.setAutoFormat(false);
  fileSystem->setConfig(fileSystemConfig);
  bool fsOK = fileSystem->begin();
  logger.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));
}

bool initialConfig = true;

void setup(void) {
  delay(100); //stabilize and settle down
  //Serial.begin(115200);
  Serial.begin(9600);
  Serial.println("");
  logger.println("Hello from Dusty!");

  initialConfig = read_config();

  setup_config();
  setup_fs();
  setup_drd_detector();

  if (initialConfig) {
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, HIGH);

    //no config, no network, configuration mode
    // 1. enable wifi accesspoint
    setup_wifi_ap();

    //setup_wifi_ap();
    // 2. show configuration web page
    setup_captive_portal();
    
  } else {
    //config exist, normal working mode
    // 1. connect to wifi
    setup_wifi();
    delay(1000);
    // 1a. check for firmware updates
    // TODO:

    // 2. show results on a web page
    setup_http_server();

    // 3. enable sensors
    setup_sensors();

    // Switching serial to talking with the SDS001
    logger.enabled = false;
    Serial.begin(9600);
  }
    
}


int lastTempTime = 0;
int lastPMTime = 0;

void loop(void) {

  if (initialConfig) {
    handle_wifi_ap();
    handle_captive_portal();    
  } else {
  MDNS.update();
  drd_loop();

  //  if (millis() - lastTempTime > 2000) {
  //    // if there's a reading ready, read it:
  //    // don't do anything until the data ready pin is high:
  //    if (digitalRead(config.dht11Pin) == HIGH) {
  //      read_temp_humid(tempAndHumid, true);
  //      // timestamp the last time you got a reading:
  //      lastTempTime = millis();
  //    }
  //  }
  //
  //
  //  if (millis() - lastPMTime > 1000) {
  //    // if there's a reading ready, read it:
  //    read_dust_data(pmData);
  //    // timestamp the last time you got a reading:
  //    lastPMTime = millis();
  //  }

    handleHttp();
  }


}
