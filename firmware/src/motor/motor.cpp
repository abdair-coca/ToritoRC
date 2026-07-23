#include <Arduino.h>
#include "pins.h"
#include "motor.h"

void MotorController::begin() {
  pinMode(PIN_MOTOR_IZQ_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IZQ_IN2, OUTPUT);
  pinMode(PIN_MOTOR_DER_IN3, OUTPUT);
  pinMode(PIN_MOTOR_DER_IN4, OUTPUT);

  analogWriteFreq(PWM_FREQ_MOTOR);
  analogWriteRange(255);
}

void MotorController::setSpeed(int left, int right) {
  _leftSpeed  = constrain(left,  -255, 255);
  _rightSpeed = constrain(right, -255, 255);

  setSingleMotor(PIN_MOTOR_IZQ_PWM, PIN_MOTOR_IZQ_IN1, PIN_MOTOR_IZQ_IN2, _leftSpeed);
  setSingleMotor(PIN_MOTOR_DER_PWM, PIN_MOTOR_DER_IN3, PIN_MOTOR_DER_IN4, _rightSpeed);
}

void MotorController::stop() {
  setSpeed(0, 0);
}

void MotorController::setSingleMotor(int pinPwm, int pinIn1, int pinIn2, int speed) {
  if (speed > 0) {
    digitalWrite(pinIn1, HIGH);
    digitalWrite(pinIn2, LOW);
    analogWrite(pinPwm, speed);
  } else if (speed < 0) {
    digitalWrite(pinIn1, LOW);
    digitalWrite(pinIn2, HIGH);
    analogWrite(pinPwm, -speed);
  } else {
    digitalWrite(pinIn1, LOW);
    digitalWrite(pinIn2, LOW);
    analogWrite(pinPwm, 0);
  }
}
