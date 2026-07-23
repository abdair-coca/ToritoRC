#pragma once

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "../protocol/protocol.h"

class NetworkManager {
public:
  void begin();
  void broadcastTelemetry(const TelemetryData& data);
  void onCommand(void (*cb)(const ControlCommand&));
  void loop();

private:
  WebSocketsServer ws{81};

  void (*commandCallback)(const ControlCommand&) = nullptr;

  static void onWsEvent(uint8_t num, WStype_t type, uint8_t* data, size_t len);
  static NetworkManager* instance;
};
