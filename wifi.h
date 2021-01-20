#ifndef _wifi_h_
#define _wifi_h_

#include <DNSServer.h>

String defaultPassword = "1234";

DNSServer dnsServer;

void setup_wifi() {
  // Connect to WiFi network

  //if !dhcp
  //wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");

  if (!config.enable_dhcp) {
    WiFi.config(config.ip, config.gw, config.mask, config.dns1, config.dns2);
  }

  WiFi.begin(config.ssid, config.password);
  logger.println("");
  String thisBoard = ARDUINO_BOARD;
  logger.println(thisBoard);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logger.print(".");
  }
  logger.println("");
  logger.print("Connected to ");
  logger.println(config.ssid);
  logger.print("IP address: ");
  logger.println(WiFi.localIP());

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("esp8266")) {
    logger.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  NBNS.begin("esp8266");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  logger.println("mDNS responder started");
}

#define DRD_ADDRESS  0
void setup_drd_detector() {
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset()) { 
    logger.println("DRD"); 
//    initialConfig = true; 
    //TODO: erase eeprom config
    //TODO: reboot
  }
}

void drd_loop() {
  drd->loop();
}

void setup_wifi_ap() {
  //bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(config.ip, config.gw, config.mask);
  WiFi.softAP(String("ESP_") + ESP.getChipId());
  while (WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
    yield();
    delay(100);
  }
  dnsServer.start(53, "*", WiFi.softAPIP()); // DNS spoofing (Only HTTP)

  logger.print("AP started. IP:");
  logger.println(WiFi.softAPIP());
  
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
}

void handle_wifi_ap() {
    dnsServer.processNextRequest();

}



#endif
