#include <Arduino.h>
#include "pins.h"
#include "audio.h"

void AudioController::begin() {
  pinMode(PIN_SPEAKER, OUTPUT);
  silence();
}

void AudioController::setRpm(int rpm) {
  _rpm = rpm;
}

void AudioController::setHorn(bool on) {
  _hornOn = on;
}

void AudioController::update(unsigned long now) {
  if (_hornOn) {
    setSpeakerFreq(440);  // A4 horn
    return;
  }

  if (_rpm <= 0) {
    silence();
    return;
  }

  // Map RPM to frequency: idle ~60Hz, max ~300Hz (moto sound)
  int freq = map(_rpm, 0, 8000, 60, 300);
  setSpeakerFreq(freq);
}

void AudioController::setSpeakerFreq(int freq) {
  if (freq == _currentFreq) return;
  _currentFreq = freq;
  analogWriteFreq(freq);
  analogWrite(PIN_SPEAKER, 128);  // 50% duty
}

void AudioController::silence() {
  _currentFreq = 0;
  analogWrite(PIN_SPEAKER, 0);
}
