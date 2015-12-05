
//#define DEBUGGING(...) Serial.println( __VA_ARGS__ )
//#define DEBUGGING_L(...) Serial.print( __VA_ARGS__ )

#include "LedStrip12V.h"

void setup() {

  Serial.begin(115200);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  WifiConnect();
  WebSocketConnect();
  MDNSConnect();
  HTTPUpdateConnect();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WifiConnect();
    WebSocketConnect();
    MDNSConnect();
  }
  else {

    webSocket.loop();
    if (millis() - lastTimeHost > 10) {
      httpServer.handleClient();
      lastTimeHost = millis();
    }
    if (millis() - lastTimeRefresh > WAIT_RAINBOW && millis() - lastTimeRefreshRainbow > rainbowDelay && rainbowFlag) {
      lastTimeRefreshRainbow = millis();
      writeWheel(cnt++, RGB);
    }
  }
}

