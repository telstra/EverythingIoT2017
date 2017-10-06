#include "Sensor.h"

void Sensor::measure()
{
  temperature = 25;
  light = 36;
}

char *Sensor::toJSON()
{
  /*  https://github.com/bblanchon/ArduinoJson/wiki/Memory-model
    Have allowed for a few extra json fields that actually being used at the moment
*/

  StaticJsonBuffer<JSON_OBJECT_SIZE(16)> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["Celsius"] = temperature;
  //root["Acc_x"] = acc_x;
  //root["Acc_y"] = acc_y;
  //root["Acc_z"] = acc_z;
  root["Light"] = light;
  root["Geo"] = geo;
  root["Schema"] = 1;

  root["Id"] = ++msgId;

  root.printTo(buffer, sizeof(buffer));

  return buffer;
}
