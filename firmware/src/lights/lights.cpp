#include <Arduino.h>
#include "pins.h"
#include "lights.h"

void LightsController::begin() {
  pinMode(PIN_FAROS,   OUTPUT);
  pinMode(PIN_INT_IZQ, OUTPUT);
  pinMode(PIN_INT_DER, OUTPUT);
  pinMode(PIN_FRENO,   OUTPUT);

  analogWriteFreq(PWM_FREQ_LIGHT);
  analogWriteRange(255);
}

void LightsController::setFrontLights(bool on) {
  _frontOn = on;
  analogWrite(PIN_FAROS, on ? 255 : 0);
}

void LightsController::setLeftBlinker(bool on) {
  _leftBlinkOn = on;
}

void LightsController::setRightBlinker(bool on) {
  _rightBlinkOn = on;
}

void LightsController::setBrake(bool on) {
  _brakeOn = on;
  analogWrite(PIN_FRENO, on ? 255 : 0);
}

void LightsController::update(unsigned long now) {
  bool blinkActive = _leftBlinkOn || _rightBlinkOn;
  if (blinkActive) {
    if (now - _lastBlinkMs >= BLINK_INTERVAL) {
      _lastBlinkMs = now;
      _blinkState = !_blinkState;
    }
  } else {
    _blinkState = false;
  }

  digitalWrite(PIN_INT_IZQ, _leftBlinkOn && _blinkState ? HIGH : LOW);
  digitalWrite(PIN_INT_DER, _rightBlinkOn && _blinkState ? HIGH : LOW);
}
