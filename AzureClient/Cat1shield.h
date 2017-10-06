#ifndef Cat1Shield_h
#define Cat1Shield_h

#include "Sensor.h"

class Cat1Shield : public Sensor
{
public:
  Cat1Shield(IoTShield *shield, const char *geo) : Sensor(shield, geo){};
  void measure();

};

#endif
