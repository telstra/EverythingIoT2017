#ifndef TimeManager_h
#define TimeManager_h

#include "iotshield.h"
#include "connection4g.h"
#include <TimeLib.h>

class TimeManager
{
public:
  TimeManager(IoTShield *shield)
  {
    this->shield = shield;
  };
  bool SetTime();

private:
  IoTShield *shield;
  int GetNumber(char *input, int len);
  time_t lastUtcUpdate;
  char isoTime[30];
  char* getISODateTime();
};

#endif
