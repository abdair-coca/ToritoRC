#include "network.h"

NetworkManager* NetworkManager::instance = nullptr;

void NetworkManager::begin() {
  instance = this;

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ToritoRC", "toritopass");

  ws.begin();
  ws.onEvent(onWsEvent);
}

void NetworkManager::broadcastTelemetry(const TelemetryData& data) {
  String json;
  serializeTelemetry(data, json);
  ws.broadcastTXT(json);
}

void NetworkManager::onCommand(void (*cb)(const ControlCommand&)) {
  commandCallback = cb;
}

void NetworkManager::loop() {
  ws.loop();
}

void NetworkManager::onWsEvent(uint8_t num, WStype_t type, uint8_t* data, size_t len) {
  if (type == WStype_TEXT && instance && instance->commandCallback) {
    data[len] = 0;
    ControlCommand cmd;
    if (parseCommand((char*)data, cmd)) {
      instance->commandCallback(cmd);
    }
  }
}
