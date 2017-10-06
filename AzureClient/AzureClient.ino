/*
  README

  PROJECT OWNER: Dave Glover | dglover@microsoft.com | @dglover

  CONTRIBUTOR: Matt Sinclair | matt.sinclair@team.telstra.com | @mjksinclair

  COPYRIGHT: Free to use, reuse and modify, no libability accepted

  CREATED: May 2017

  UPDATED: October 2017 

  SUMMARY:
  This project was implemented and tested on the Arduino 101 with a Cat-1 Shield.

  See https://github.com/telstra/TIC2017/wiki/1.-The-Kit

  This solution streams sample sensor data securely over HTTPS / REST directly to Azure IoT Hub or Azure Event Hub.

  PLATFORM: Arduino 101 with a Cat-1 Shield

  REQUIRED LIBRARIES: Install Arduino Libraries from Sketch -> Include Library -> Manage Libraries
  - ArduinoJson
  - Time

*/

#include "iotshield.h"
#include "shieldinterface.h"
#include "connection4g.h"
#include "AzureIotHub.h"
#include "Sensor.h"
#include "Cat1shield.h"
#include "TimeManager.h"
#include "MemoryFree.h"

const char *connectionString = "YOUR CONNECTION STRING HERE";
const char *deviceLocation = "Sydney";
int publishRateInSeconds = 10;

ShieldInterface shieldif;
IoTShield shield(&shieldif);
Connection4G conn(true, &shieldif);
TimeManager timeMgr(&shield);

AzureIotHub hub(&conn, connectionString, "sending-data-v1");
Cat1Shield sensor(&shield, deviceLocation);

String data;
int contentLength = 0;
int msgId = 0;
char lightString[15];

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  delay(1000);

//  while (!Serial) ; // Wait for serial character before starting

  Serial.println(F("******* Waiting for shield *********"));
  shield.waitUntilShieldIsReady();
  Serial.println(F("******* Shield ready *********"));
  
}

void loop()
{
  if (shield.isPDPContextActive())
  {
    if (timeMgr.SetTime())
    {

        Serial.println(F("******* Reading Measurements *********"));
        collectMeasurements();
      
        Serial.println(F("******* Sending Measurement *********"));
        String result = hub.send(sensor.toJSON());
        
        Serial.println(F("******* Response *********"));
        Serial.print(result); // response 204 means successful send of data to Azure IoT Hub
    }
    else
    {
      Serial.println("Date/Time not set");
    }
    delay(publishRateInSeconds * 1000); // limit publishing rate
  }
  else
  {
    Serial.println(F("[FAIL] Shield is not connected to 4G network!"));
    delay(5000);
  }
}

void collectMeasurements() {
        sensor.measure();
        Serial.print(F("Temp: "));
        Serial.println(sensor.temperature);
        Serial.print(F("Light: "));
        Serial.println(sensor.light);
}



/*String sendJSensorPayload(const char *image, int id)
{
  int startChar = 0;
  time_t start;

  String jsonStart = "{\"ImageId\":\"" + String(id) + "\", \"Mem\":" + String(freeMemory()) + ",\"MsgId\":" + String(msgId++) + ",\"Schema\":\"Image\",\"Image\":\"";
  String jsonEnd = "\"}";

  int imageLength = strlen(image);
  Serial.print("Image Size: ");
  Serial.println(imageLength);
  
  start = now();

  contentLength = jsonStart.length();
  contentLength += jsonEnd.length();
  contentLength += imageLength;

  hub.sendBegin(contentLength);
  hub.sendData(jsonStart);

  // now stream the image in segmentLength byte chunks
  while (startChar + segmentLength < imageLength)
  {
    hub.sendData(image + startChar, segmentLength);
    startChar += segmentLength;
  }

  // send remaining bytes in image
  hub.sendData(image + startChar, imageLength - startChar);
  hub.sendData(jsonEnd);
  
  String result = "Response: " + hub.sendEnd();

  result += ", Seconds: " + String(now() - start);

  return result;
}*/
