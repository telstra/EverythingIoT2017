#include "AzureIotHub.h"

AzureIotHub::AzureIotHub(Connection4G *conn, const char *cs, const char *route)
{
  this->conn = conn;
  this->route = route;
  AzureIotHub::setConnectionString(cs);
  AzureIotHub::initialiseHub();
}

void AzureIotHub::initialiseHub()
{
  sasUrl = urlEncode(host) + urlEncode(TARGET_URL) + (String)deviceId;
  endPoint = (String)TARGET_URL + (String)deviceId + (String)IOT_HUB_END_POINT;
  deviceBoundEndPoint = (String)TARGET_URL + (String)deviceId + (String)IOT_HUB_DEVICE_BOUND;
}

String AzureIotHub::urlEncode(const char *msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0')
  {
    if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9'))
    {
      encodedMsg += *msg;
    }
    else
    {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}

String AzureIotHub::createSas(char *key, String url)
{
  sasExpiryTime = now() + sasExpiryPeriodInSeconds;
  String stringToSign = url + "\n" + sasExpiryTime;

  // START: Create signature
  // https://raw.githubusercontent.com/adamvr/arduino-base64/master/examples/base64/base64.ino

  int keyLength = strlen(key);

  int decodedKeyLength = base64_dec_len(key, keyLength);
  char decodedKey[decodedKeyLength]; //allocate char array big enough for the base64 decoded key

  base64_decode(decodedKey, key, keyLength); //decode key

  Sha256.initHmac((const uint8_t *)decodedKey, decodedKeyLength);
  Sha256.print(stringToSign);
  char *sign = (char *)Sha256.resultHmac();
  // END: Create signature

  // START: Get base64 of signature
  int encodedSignLen = base64_enc_len(HASH_LENGTH);
  char encodedSign[encodedSignLen];
  base64_encode(encodedSign, sign, HASH_LENGTH);

  // SharedAccessSignature
  return "sr=" + url + "&sig=" + urlEncode(encodedSign) + "&se=" + sasExpiryTime;
  // END: create SAS
}

String AzureIotHub::buildHttpRequest(int length)
{
  return "POST " + endPoint + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "Authorization: SharedAccessSignature " + fullSas + "\r\n" +
         "Content-Type: application/atom+xml;type=entry;charset=utf-8\r\n" +
         "iothub-app-route-id: " + route + "\r\n" +
         "Content-Length: " + String(length) + "\r\n\r\n";
}

String AzureIotHub::buildDeviceBoundHttpRequest()
{
  return "GET " + deviceBoundEndPoint + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "Authorization: SharedAccessSignature " + fullSas + "\r\n" +
         "Connection: close" +
         "\r\n\r\n";
}

const char *AzureIotHub::GetStringValue(String value)
{
  int len = value.length() + 1;
  char *temp = new char[len];
  value.toCharArray(temp, len);
  return temp;
}

bool AzureIotHub::renewSas()
{
  if (timeStatus() == timeNotSet)
  {
    return false;
  }

  if (now() > sasExpiryTime)
  {
    delete[] fullSas;
    fullSas = (char *)GetStringValue(createSas(key, sasUrl));
  }
  return true;
}

bool AzureIotHub::connectToAzure()
{
  if (connected4G)
  {
    return true;
  }
  conn->openTCP(host, 443);
  delay(1000); //network settle time
  connected4G = true;
  return true;
}

String AzureIotHub::requestDeviceBoundMessage()
{
  if (!renewSas())
  {
    return "Time not set";
  }

  connectToAzure();

  String request = buildDeviceBoundHttpRequest();
  Serial.print(request);
  request.toCharArray(requestBuffer, 512);

  conn->TCPWrite(requestBuffer, request.length());
  delay(750);

  int responseLength = conn->TCPRead(requestBuffer, 512);

  Serial.print("request length: ");
  Serial.println(responseLength);

  return String(requestBuffer);
}

String AzureIotHub::send(String json)
{
  AzureIotHub::sendBegin(json.length());
  AzureIotHub::sendData(json);
  return AzureIotHub::sendEnd();
}

void AzureIotHub::sendBegin(int length)
{
  if (!renewSas())
  {
    return;
  }

  connectToAzure();

  String request = buildHttpRequest(length);
  AzureIotHub::sendData(request);
}

void AzureIotHub::sendData(const char *data, int length)
{
  int segmentLength = 200;
  int startChar = 0;

  if (length < 0)
  {
    return;
  }

  if (length < segmentLength)
  {
    strncpy(requestBuffer, data, length);
    requestBuffer[length] = 0;
    conn->TCPWrite(requestBuffer, length);
  }
  else
  {
    while (startChar + segmentLength < length)
    {
      strncpy(requestBuffer, data + startChar, segmentLength);
      requestBuffer[segmentLength] = 0;
      conn->TCPWrite(requestBuffer, segmentLength);
      startChar += segmentLength;
    }
    
    length -= startChar;
    strncpy(requestBuffer, data + startChar, length);
    requestBuffer[length] = 0;
    conn->TCPWrite(requestBuffer, length);
  }
}

void AzureIotHub::sendData(String data)
{
  sendData(data.c_str(), data.length());
}

String AzureIotHub::sendEnd()
{
  // delay(750);
  int responseLength = 0, retryCount = 0;

  do
  {
    delay(50);
    responseLength = conn->TCPRead(requestBuffer, 512);
  } while (responseLength == 0 && retryCount++ < 10);

  String response = "";

  if (responseLength > 12)
  {
    for (int c = 9; c < 12; c++)
    {
      response += requestBuffer[c];
    }
  }
  else
  {
    response = "unknown";
  }

  //  conn->closeTCP();
  return response;
}

void AzureIotHub::setConnectionString(String cs)
{
  host = GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 0), '=', 1));
  deviceId = GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 1), '=', 1));
  key = (char *)GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 2), '=', 1));
}

// http://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
String AzureIotHub::splitStringByIndex(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
