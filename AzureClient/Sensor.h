#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson - installed via library manager
#include "iotshield.h"

class Sensor
{
public:
  Sensor(IoTShield *shield, const char *geo)
  {
    this->geo = geo;
    this->shield = shield;
  };

  void measure();
  char *toJSON();

  float temperature;
  float humidity;
  int pressure;
  int light;
  int msgId;
  const char *geo;

protected:
  bool initialised;
  IoTShield *shield;

private:
  char buffer[256];
};

#endif
