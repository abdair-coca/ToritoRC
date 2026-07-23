#include "network.h"

NetworkManager* NetworkManager::instance = nullptr;

static String getContentType(const String& path) {
  if (path.endsWith(".html")) return "text/html";
  else if (path.endsWith(".css")) return "text/css";
  else if (path.endsWith(".js")) return "application/javascript";
  else if (path.endsWith(".json")) return "application/json";
  else if (path.endsWith(".svg")) return "image/svg+xml";
  else if (path.endsWith(".glb")) return "model/gltf-binary";
  else if (path.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

static bool handleFileRead(ESP8266WebServer& server, String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  
  // SPA Fallback to index.html
  if (LittleFS.exists("/index.html")) {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
    return true;
  }
  
  return false;
}

void NetworkManager::begin() {
  instance = this;

  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("ToritoRC", "toritopass");

  LittleFS.begin();

  // Serve static files from LittleFS
  server.onNotFound([this]() {
    if (!handleFileRead(server, server.uri())) {
      server.send(404, "text/plain", "404: Not Found");
    }
  });

  server.begin();

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
  server.handleClient();
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
