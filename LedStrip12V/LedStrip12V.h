#define REFRESH_RAINBOW 500
#define WAIT_RAINBOW 10000
#define BLUEPIN 13
#define REDPIN 12
#define GREENPIN 14
#define SSID_ME "YourSSID"
#define PW_ME "YourPW"
#define HOST_ME "esp8266Color"

#ifndef DEBUGGING
#define DEBUGGING(...)
#endif
#ifndef DEBUGGING_L
#define DEBUGGING_L(...)
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Arduino.h>
#include <WebSocketsServer.h>
#include <Hash.h>

//Globals
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* host = HOST_ME;
const char* ssid     = SSID_ME;
const char* password = PW_ME;
unsigned long lastTimeHost = 0;
unsigned long lastTimeRefresh = 0;
unsigned long lastTimeRefreshRainbow = 0;
boolean rainbowFlag = 1;
int RGB[3];
int cnt = 0;
int rainbowDelay = REFRESH_RAINBOW;

//Wheel - return HUE[iii] in RGB
void Wheel(int WheelPos, int* RGB) {
  WheelPos = WheelPos % 256;

  if (WheelPos < 85) {
    RGB[0] = WheelPos * 3;
    RGB[1] = 255 - WheelPos * 3;
    RGB[2] = 0;
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    RGB[2] = WheelPos * 3;
    RGB[0] = 255 - WheelPos * 3;
    RGB[1] = 0;
  }
  else if (WheelPos < 255) {
    WheelPos -= 170;
    RGB[1] = WheelPos * 3;
    RGB[2] = 255 - WheelPos * 3;
    RGB[0] = 0;

  }
  else
  {
    WheelPos -= 255;
    RGB[0] = WheelPos * 3;
    RGB[1] = 255 - WheelPos * 3;
    RGB[2] = 0;
  }
}

// Write wheel to leds
void writeWheel(int WheelPos, int* RGB) {
  Wheel(WheelPos, RGB);
  analogWrite(REDPIN, map(RGB[0], 0, 255, 0, 1023));
  analogWrite(GREENPIN, map(RGB[1], 0, 255, 0, 1023));
  analogWrite(BLUEPIN, map(RGB[2], 0, 255, 0, 1023));
}


// WebSOcket Events
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
      }
      break;
      
    case WStype_TEXT:
      {
        lastTimeRefresh = millis();

        String text = String((char *) &payload[0]);
        if (text == "LED") {
          digitalWrite(13, HIGH);
          delay(500);
          digitalWrite(13, LOW);
          Serial.println("led just lit");
          webSocket.sendTXT(num, "led just lit", length);
        }

        if (text.startsWith("x")) {
          String xVal = (text.substring(text.indexOf("x") + 1, text.length()));
          int xInt = xVal.toInt();
          analogWrite(REDPIN, xInt);
          DEBUGGING(xVal);
        }

        if (text.startsWith("y")) {
          String yVal = (text.substring(text.indexOf("y") + 1, text.length()));
          int yInt = yVal.toInt();
          analogWrite(GREENPIN, yInt);
          DEBUGGING(yVal);
        }

        if (text.startsWith("z")) {
          String zVal = (text.substring(text.indexOf("z") + 1, text.length()));
          int zInt = zVal.toInt();
          analogWrite(BLUEPIN, zInt);
          DEBUGGING(zVal);
        }
        if (text.startsWith("t")) {
          String tVal = (text.substring(text.indexOf("t") + 1, text.length()));
          rainbowDelay = tVal.toInt();
          lastTimeRefreshRainbow = 0;
          lastTimeRefresh = 0;
          DEBUGGING(tVal);        
        }        
        if (text == "RESET") {
          rainbowFlag = 0;
          analogWrite(BLUEPIN, LOW);
          analogWrite(REDPIN, LOW);
          analogWrite(GREENPIN, LOW);
          DEBUGGING("reset");

        }
        if (text == "RAINBOW") {
          rainbowFlag = 1;
          lastTimeRefreshRainbow = 0;
          lastTimeRefresh = 0;
       /*   for (int iii = 0; iii < 256; iii++) {
            writeWheel(iii, RGB);
            delay(10);
          }*/
          DEBUGGING("rainbow");
        }
      }
      break;

    case WStype_BIN:
      hexdump(payload, length);
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
  }
}


// Wifi Connection
void WifiConnect() {

 // WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  DEBUGGING("Connected");
  DEBUGGING(WiFi.localIP());

}

// WebSocket Connection
void WebSocketConnect() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

// MDNS 
void MDNSConnect() {
  if (!MDNS.begin(host)) {
   DEBUGGING("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  DEBUGGING("mDNS responder started");
  MDNS.addService("ws", "tcp", 81);
  MDNS.addService("http", "tcp", 80);
}

// HTTP updater connection
void HTTPUpdateConnect() {
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  DEBUGGING_L("HTTPUpdateServer ready! Open http://");
  DEBUGGING_L(host);
  DEBUGGING(".local/update in your browser\n");
}


