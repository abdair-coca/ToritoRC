#pragma once

#define GEAR_NEUTRAL     0
#define GEAR_REVERSE     6
#define GEAR_MAX         5
#define GEAR_COUNT       7

struct GearRatios {
  float maxSpeed;   // multiplier 0..1
  int   maxRpm;     // max engine RPM for this gear
  const char* name;
};

class Transmission {
public:
  void begin();
  void setGear(int gear);    // 0=N, 1-5, 6=R
  int  getGear() const { return _currentGear; }
  int  getRpm()   const { return _rpm; }
  float getSpeedFactor() const;  // 0..1

  void update(float throttle, float dt);  // throttle 0..1

  int gearUp();
  int gearDown();

private:
  int _currentGear = 0;   // N
  int _rpm         = 0;   // 0..maxRpm

  static constexpr int MAX_RPM = 8000;

  static constexpr GearRatios GEARS[7] = {
    {0.00f, 0,    "N"},
    {0.20f, 3000, "1"},
    {0.35f, 4000, "2"},
    {0.50f, 5000, "3"},
    {0.70f, 6000, "4"},
    {1.00f, 7000, "5"},
    {0.15f, 2000, "R"},
  };
};
