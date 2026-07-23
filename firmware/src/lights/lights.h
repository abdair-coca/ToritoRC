#pragma once

class LightsController {
public:
  void begin();
  void setFrontLights(bool on);
  void setLeftBlinker(bool on);
  void setRightBlinker(bool on);
  void setBrake(bool on);

  void update(unsigned long now);

  bool frontLightsOn()    const { return _frontOn; }
  bool leftBlinkerOn()    const { return _leftBlinkOn && _blinkState; }
  bool rightBlinkerOn()   const { return _rightBlinkOn && _blinkState; }
  bool brakeOn()          const { return _brakeOn; }

private:
  bool _frontOn     = false;
  bool _leftBlinkOn = false;
  bool _rightBlinkOn= false;
  bool _brakeOn     = false;
  bool _blinkState  = false;
  unsigned long _lastBlinkMs = 0;

  static constexpr unsigned long BLINK_INTERVAL = 300; // ms
};
