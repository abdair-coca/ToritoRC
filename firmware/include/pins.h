#pragma once

// D1 Mini GPIO mapping
#define PIN_FAROS        D0   // GPIO16 - Forward lights (PWM sw)
#define PIN_INT_IZQ      D1   // GPIO5  - Left blinkers (front+rear)
#define PIN_INT_DER      D2   // GPIO4  - Right blinkers (front+rear)
#define PIN_FRENO        D3   // GPIO0  - Brake light (PWM sw)
#define PIN_SPEAKER      D4   // GPIO2  - Audio output (PWM sw)

#define PIN_MOTOR_IZQ_PWM  D5 // GPIO14 - Left motor speed (hw PWM)
#define PIN_MOTOR_DER_PWM  D6 // GPIO12 - Right motor speed (hw PWM)
#define PIN_MOTOR_IZQ_IN1  D7 // GPIO13 - Left motor dir A
#define PIN_MOTOR_IZQ_IN2  D8 // GPIO15 - Left motor dir B
#define PIN_MOTOR_DER_IN3  RX // GPIO3  - Right motor dir A
#define PIN_MOTOR_DER_IN4  TX // GPIO1  - Right motor dir B

#define PIN_BATTERY      A0   // ADC - Battery voltage divider

#define PWM_FREQ_MOTOR   1000
#define PWM_FREQ_LIGHT   5000
#define PWM_RES_MOTOR    10   // 0-1023
#define PWM_RES_LIGHT    8    // 0-255

#define CH_MOTOR_IZQ     0
#define CH_MOTOR_DER     1

// Gear constants moved to transmission.h
