#include <Arduino.h>
#include "transmission.h"

constexpr GearRatios Transmission::GEARS[7];

void Transmission::begin() {
  _currentGear = 0;
  _rpm = 0;
}

void Transmission::setGear(int gear) {
  if (gear >= 0 && gear <= 6) {
    _currentGear = gear;
  }
}

int Transmission::gearUp() {
  if (_currentGear == 0) {
    _currentGear = 1;  // N -> 1
  } else if (_currentGear < GEAR_MAX) {
    _currentGear++;
  }
  return _currentGear;
}

int Transmission::gearDown() {
  if (_currentGear == 0) {
    // N down -> nothing
  } else if (_currentGear == 1) {
    _currentGear = 0;  // N
  } else {
    _currentGear--;
  }
  return _currentGear;
}

float Transmission::getSpeedFactor() const {
  if (_currentGear == 0) return 0;
  return GEARS[_currentGear].maxSpeed;
}

void Transmission::update(float throttle, float dt) {
  if (_currentGear == 0) {
    _rpm = 0;
    return;
  }

  int targetRpm = (int)(throttle * GEARS[_currentGear].maxRpm);
  targetRpm = constrain(targetRpm, 0, MAX_RPM);

  float lerp = 5.0f * dt;
  _rpm += (int)((targetRpm - _rpm) * lerp);
  _rpm = constrain(_rpm, 0, MAX_RPM);
}
