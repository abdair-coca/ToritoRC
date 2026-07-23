#include <Arduino.h>
#include "pins.h"
#include "motor/motor.h"
#include "transmission/transmission.h"
#include "lights/lights.h"
#include "audio/audio.h"
#include "network/network.h"
#include "protocol/protocol.h"

MotorController    motors;
Transmission       transmission;
LightsController   lights;
AudioController    audio;
NetworkManager     network;

ControlCommand     currentCmd;
TelemetryData      telemetry;
unsigned long      lastTelemetryMs = 0;
unsigned long      lastLoopMs      = 0;

void onCommandReceived(const ControlCommand& cmd) {
  currentCmd = cmd;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  motors.begin();
  transmission.begin();
  lights.begin();
  audio.begin();

  // Set gear from command
  transmission.setGear(currentCmd.gear);

  network.onCommand(onCommandReceived);
  network.begin();

  lastLoopMs = millis();
}

void loop() {
  unsigned long now = millis();
  float dt = (now - lastLoopMs) / 1000.0f;
  if (dt > 0.1f) dt = 0.1f;  // cap
  lastLoopMs = now;

  network.loop();

  // --- Transmission ---
  transmission.setGear(currentCmd.gear);
  float throttle = max(abs(currentCmd.leftMotor), abs(currentCmd.rightMotor)) / 255.0f;
  transmission.update(throttle, dt);

  // --- Motors ---
  float speedFactor = transmission.getSpeedFactor();
  int leftSpeed  = currentCmd.leftMotor  * speedFactor;
  int rightSpeed = currentCmd.rightMotor * speedFactor;
  motors.setSpeed(leftSpeed, rightSpeed);

  // --- Lights ---
  lights.setFrontLights(currentCmd.frontLights);
  lights.setLeftBlinker(currentCmd.leftBlinker);
  lights.setRightBlinker(currentCmd.rightBlinker);
  lights.setBrake(currentCmd.brake);
  lights.update(now);

  // --- Audio ---
  audio.setRpm(transmission.getRpm());
  audio.setHorn(currentCmd.horn);
  audio.update(now);

  // --- Telemetry broadcast (10 Hz) ---
  if (now - lastTelemetryMs >= 100) {
    lastTelemetryMs = now;
    telemetry.batteryVoltage = analogRead(PIN_BATTERY) * (3.3f / 1024.0f) * 2.0f; // voltage divider
    telemetry.speedMps = speedFactor * transmission.getRpm() * 0.001f;
    telemetry.gear     = transmission.getGear();
    telemetry.rpm      = transmission.getRpm();
    telemetry.frontLights = lights.frontLightsOn();
    telemetry.leftBlinker = lights.leftBlinkerOn();
    telemetry.rightBlinker= lights.rightBlinkerOn();
    telemetry.brake    = lights.brakeOn();
    network.broadcastTelemetry(telemetry);
  }
}
