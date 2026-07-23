#pragma once

#include <ArduinoJson.h>

struct ControlCommand {
  int leftMotor  = 0;   // -255..255
  int rightMotor = 0;   // -255..255
  int gear       = 0;   // 0=N, 1-5, 6=R
  bool frontLights = false;
  bool leftBlinker  = false;
  bool rightBlinker = false;
  bool brake        = false;
  bool horn         = false;
};

struct TelemetryData {
  float batteryVoltage = 0;
  float speedMps       = 0;
  int   gear           = 0;
  int   rpm            = 0;
  bool  frontLights    = false;
  bool  leftBlinker    = false;
  bool  rightBlinker   = false;
  bool  brake          = false;
};

bool parseCommand(const char* json, ControlCommand& cmd);
void serializeTelemetry(const TelemetryData& data, String& out);
