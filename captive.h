#ifndef _captive_h_
#define _captive_h_

#include "PageBuilder.h"
bool reconnection_required = false;

// Convert RSSI dBm to signal strength
unsigned int toWiFiQuality(int32_t rssi) {
  unsigned int  qu;
  if (rssi <= -100)
    qu = 0;
  else if (rssi >= -50)
    qu = 100;
  else
    qu = 2 * (rssi + 100);
  return qu;
}

// This callback function would be invoked from the root page and scans nearby
// WiFi-AP to make a connectable list.
String listSSID(PageArgument& args) {
  String s_ssid_list = "";
  int8_t nn = WiFi.scanNetworks(false, true);
  for (uint8_t i = 0; i < nn; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0)
      ssid = "?";
    s_ssid_list += "<div class=\"ssid_list\"><a href=\"/join?ssid=";
    s_ssid_list += ssid == "?" ? "%3F" : ssid;
    s_ssid_list += "&psk_type=" + String(WiFi.encryptionType(i)) + "\">" + ssid + "</a>";
    s_ssid_list += String(toWiFiQuality(WiFi.RSSI(i))) + "%";
    if (WiFi.encryptionType(i) != ENC_TYPE_NONE)
      s_ssid_list += "<span class=\"img_lock\" />";
    s_ssid_list += "</div>";
  }
  reconnection_required = false;
  return s_ssid_list;
}

// HTML page declarations.
#define TITLE "Dusty"
#define SUBTITLE "Dust and temperature sensor page"

PageElement HEAD_ELM("file:/head.html", {
  { "TITLE", [](PageArgument& args){return TITLE;} },
  { "SUBTITLE", [](PageArgument& args){return SUBTITLE;} },
  { "SSID", [](PageArgument& args){return config.ssid;} }
});
PageElement FOOT_ELM("file:/foot.html", {
  { "VERSION", [](PageArgument& args){return config.version;} }
});


// root page
PageElement SSID_ELM("file:/captive.root.html", {
  { "SSID_LIST", listSSID },
  { "URI_ROOT", [](PageArgument& args) { return "/"; }}
});
PageBuilder SSID_PAGE("/", {HEAD_ELM, SSID_ELM, FOOT_ELM });

// Connection failed page
PageElement FAILED_ELM("file:/captive.failed.html", {
  { "SSID", [](PageArgument& args) { return config.ssid; } },
  { "RESULT", [](PageArgument& args) { return String(WiFi.status()); } }
});
PageBuilder FAILED_PAGE("/failed", {HEAD_ELM, FAILED_ELM, FOOT_ELM});

// Form with SSID & Password
PageElement FORM_ELM("file:/config.form.html", {  
  { "SSID", [](PageArgument& args) { return (String(config.ssid).length()==0)?config.ssid:"SSID"; } },
  { "STATIC_IP", [](PageArgument& args) { return (!config.enable_dhcp)?config.ip.toString():"Static IP"; } },
  { "GW", [](PageArgument& args) { return (!config.enable_dhcp)?config.gw.toString():"Gateway"; } },
  { "MASK", [](PageArgument& args) { return (!config.enable_dhcp)?config.mask.toString():"Subnet mask"; } },

  { "DNS1", [](PageArgument& args) { return config.dns1.toString(); } },
  { "DNS2", [](PageArgument& args) { return config.dns2.toString(); } }
});
PageBuilder FORM_PAGE("/config", {HEAD_ELM, FORM_ELM, FOOT_ELM});

// Connection successful page
PageElement WELCOME_ELM("file:/captive.welcome.html", {
  { "SSID", [](PageArgument& args) { return WiFi.SSID(); } },
  { "IP", [](PageArgument& args) { return WiFi.localIP().toString(); } },
  { "GATEWAY", [](PageArgument& args) { return WiFi.gatewayIP().toString(); } },
  { "SUBNET", [](PageArgument& args) { return  WiFi.subnetMask().toString(); } },
  { "ESP_CORE_VER", [](PageArgument& args){return ESP.getCoreVersion(); } },
  { "ESP_SDK_VER", [](PageArgument& args){return ESP.getSdkVersion(); } }
});
PageBuilder WELCOME_PAGE("/connected", {HEAD_ELM, WELCOME_ELM, FOOT_ELM});


void BLINK() { // The internal LED will blink 5 times when a password is received.
  int count = 0;
  while(count < 5){
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    count = count + 1;
  }
}

// Accepting connection request.
// Flag the occurrence of the connection request and response a redirect to
// the connection result page.
String reqConnect(PageArgument& args) {
  if (args.hasArg("ssid") && args.hasArg("password")) {
  strncpy(config.ssid, args.arg("ssid").c_str(), WIFI_SSID_MAX_LEN);
  strncpy(config.password, args.arg("password").c_str(), WIFI_PASS_MAX_LEN);
  } else {
    //TODO: error handling
  }

  if (args.hasArg("static_ip")) {
    config.ip.fromString(args.arg("static_ip")); 
  }
  if (args.hasArg("static_gw")) {
    config.gw.fromString(args.arg("static_gw")); 
  }
  if (args.hasArg("static_mask")) {
    config.mask.fromString(args.arg("static_mask")); 
  }
  if (args.hasArg("dns1")) {
    config.dns1.fromString(args.arg("dns1")); 
  }
  if (args.hasArg("dns2")) {
    config.dns2.fromString(args.arg("dns2")); 
  }

  // Leave from the AP currently.
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    while (WiFi.status() == WL_CONNECTED);
  }

  // save the config to the memory
  store_config();
  BLINK();
  
  // Available upon connection request.
  //CONNECT_REQ = true;
  //REDIRECT_URI = "";
  //CURRENT_HOST = "";

  String redirect_uri = "/welcome";

  // Redirect http request to connection result page.
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + String(redirect_uri), true);
  server.send(302, "text/plain", "");
  FORM_PAGE.cancel();
  return "";
}

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  logger.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (fileSystem->exists(path)) {                        // If the file exists
    File file = fileSystem->open(path, "r");             // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  logger.println("\tFile Not Found");
  return false;     
}

void setup_captive_portal() {  

  SSID_PAGE.insert(server);
  FORM_PAGE.insert(server);
  WELCOME_PAGE.insert(server);
  FAILED_PAGE.insert(server);
  
  server.onNotFound([]() { 
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/html", "404: not found"); 
    }
  });
  server.begin();
  
  //TODO: build captive web page
  // get password
  // save config
  // reset
  // ESP.restart();

}

void handle_captive_portal() {
  server.handleClient();
}

#endif //_captive_h_
