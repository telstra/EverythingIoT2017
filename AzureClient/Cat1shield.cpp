#include "Cat1shield.h"

void Cat1Shield::measure()
{
  temperature = pressure = humidity = 0;
  char temp[20];
  char lum[15];

  shield->getTemperature(temp);
  temperature = atof(temp);

  shield->getLightLevel(lum);
  light = atoi(lum);
}
