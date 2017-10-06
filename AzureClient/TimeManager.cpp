#include "TimeManager.h"

bool TimeManager::SetTime()
{
  if (lastUtcUpdate + (5 * 60) > now() && timeStatus() == timeSet)
  {
    return true;
  }

  // this is a hack work around as device not reciving the correct time from the cellur service
  // this will be replaced with a http get to an epock time service

  //  setTime(00, 00, 00, 04, 05, 2017);

  if (shield->isPDPContextActive())
  {
    delay(2000);
    char timeBuffer[30];
    shield->getTime(timeBuffer);
    //2017-05-05T11:11:52.000+10:00

    Serial.println(timeBuffer);

    int year = TimeManager::GetNumber(timeBuffer, 4);
    int month = TimeManager::GetNumber(timeBuffer + 5, 2);
    int day = TimeManager::GetNumber(timeBuffer + 8, 2);
    int hour = TimeManager::GetNumber(timeBuffer + 11, 2);
    int minute = TimeManager::GetNumber(timeBuffer + 14, 2);
    int second = TimeManager::GetNumber(timeBuffer + 17, 2);
    int timeZoneHours = TimeManager::GetNumber(timeBuffer + 24, 2);
    int timeZoneMins = TimeManager::GetNumber(timeBuffer + 27, 2);
    int timeZoneSeconds = ((timeZoneHours * 60) + timeZoneMins) * 60;
    char timeZoneAdjust = timeBuffer[23];
    //     Serial.println(timeZoneHours);
    //     Serial.println(timeZoneMins);
    //     Serial.println(timeZoneSeconds);
    //     Serial.println(plusMinus);

    // check that the date/time is correct and not the default
    if (day == 1 && month == 1 && year == 2010)
    {
      delay(5000);
      return false;
    }

    setTime(hour, minute, second, day, month, year);
    if (timeZoneAdjust == '+')
    {
      adjustTime(-timeZoneSeconds);
    }
    else
    {
      adjustTime(timeZoneSeconds);
    }

    lastUtcUpdate = now();

    Serial.println(TimeManager::getISODateTime());

    return true;
  }
  return false;
}

int TimeManager::GetNumber(char *input, int len)
{
  char number[20];
  if (len > 20)
  {
    return 0;
  }
  strncpy(number, input, len);
  number[len] = '\0';
  int value = atoi(number);
  return value;
}

char *TimeManager::getISODateTime()
{
  sprintf(isoTime, "%4d-%02d-%02dT%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
  return isoTime;
}
