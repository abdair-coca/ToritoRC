#pragma once

class MotorController {
public:
  void begin();
  void setSpeed(int left, int right);   // -255..255
  void stop();
  int  getLeftSpeed()  const { return _leftSpeed; }
  int  getRightSpeed() const { return _rightSpeed; }

private:
  int _leftSpeed  = 0;
  int _rightSpeed = 0;

  void setSingleMotor(int pinPwm, int pinIn1, int pinIn2, int speed);
};
