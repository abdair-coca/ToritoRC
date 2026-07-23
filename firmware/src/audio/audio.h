#pragma once

class AudioController {
public:
  void begin();
  void setRpm(int rpm);
  void setHorn(bool on);
  void update(unsigned long now);

private:
  int  _rpm    = 0;
  bool _hornOn = false;
  int  _currentFreq = 0;

  void setSpeakerFreq(int freq);
  void silence();
};
