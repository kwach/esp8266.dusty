#ifndef _http_h_
#define _http_h_

/*
 * HTTP things
 */
#include "PageBuilder.h"

String tokenFunc(PageArgument&);
PageElement elm("{{RES}}", {{ "RES", tokenFunc }});
PageBuilder page("/", { elm });   // Accessing "/" will call the tokenFunc.
PageElement elmHello("hello, world");
PageBuilder pageHello("/hello", { elmHello });

// The tokenFunc returns no HTML and sends redirection-code as
// particular response by an owned http header.
String tokenFunc(PageArgument& args) {

  // When accessing "/", it sends its own http response so that
  // it will be transferred to "/hello".
  server.sendHeader("Location", "/hello", true);
  server.sendHeader("Connection", "keep-alive");
  server.send(302, "text/plain", "");
  server.client().stop();

  // The cancel method notifies sending stop to the PageBuilder.
  page.cancel();
  return "";
}

void setup_http_server() {
  
  
  // Start TCP (HTTP) server
  //LEDPage.authentication("admin", "admin", DIGEST_AUTH, "dusty");
  page.insert(server);
  pageHello.insert(server);
  server.begin();
  logger.print("TCP server started on: "); logger.println(WiFi.localIP());
}


void handleRoot() {
  IPAddress ip = WiFi.localIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

  String s;
  s = "Dusty welcomes you.\r\n";
  s += ipStr;
  s += "Data: \r\n";
  s += "DH11 status: " + tempAndHumid.status ? "OK\r\n":"FAIL\r\n";
  s += "Humidity: "; s += tempAndHumid.humidity; s += "%\r\n";
  s += "Temperature: "; s += tempAndHumid.temperatureC; s += " C\r\n";
  s += "Heat Index: "; s += tempAndHumid.heatIndexC; s += " C\r\n";

  s += "Dust: "; s+= pmData.status;  s += "\r\n";
  s += "PM 10um: "; s += pmData.pm10; s += "\r\n";
  s += "PM 2.5um: "; s += pmData.pm25; s += "\r\n";
  
  server.send(200, "text/plain", s);
}

void handleHttp() {
  server.handleClient();
}

#endif
