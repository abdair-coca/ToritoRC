#include "protocol.h"

bool parseCommand(const char* json, ControlCommand& cmd) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) return false;

  cmd.leftMotor   = doc["leftMotor"]  | 0;
  cmd.rightMotor  = doc["rightMotor"] | 0;
  cmd.gear        = doc["gear"]       | 0;
  cmd.frontLights = doc["frontLights"]| false;
  cmd.leftBlinker = doc["leftBlinker"]| false;
  cmd.rightBlinker= doc["rightBlinker"]| false;
  cmd.brake       = doc["brake"]      | false;
  cmd.horn        = doc["horn"]       | false;

  return true;
}

void serializeTelemetry(const TelemetryData& data, String& out) {
  JsonDocument doc;
  doc["battery"]    = data.batteryVoltage;
  doc["speed"]      = data.speedMps;
  doc["gear"]       = data.gear;
  doc["rpm"]        = data.rpm;
  doc["frontLights"]= data.frontLights;
  doc["leftBlinker"]= data.leftBlinker;
  doc["rightBlinker"]= data.rightBlinker;
  doc["brake"]      = data.brake;
  serializeJson(doc, out);
}
