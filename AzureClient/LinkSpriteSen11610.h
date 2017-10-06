#ifndef LinkSpriteSen1160_h
#define LinkSpriteSen1160_h

#include <SoftwareSerial.h>
#include <Arduino.h>

class LinkSprite
{
public:
  LinkSprite(int cameraRecievePin, int cameraTransmitPin)
  {
    camSerial = new SoftwareSerial(cameraRecievePin, cameraTransmitPin); // RX, TX
    camSerial->begin(38400);

  }
  void SendResetCmd();
  void SetImageSizeCmd(byte Size);
  void SetBaudRateCmd(byte baudrate);
  void SendTakePhotoCmd();
  int SendReadDataCmd(int cameraAddress);
  void SendTakePhotoCmd(boolean extraDebugFlag);
  void StopTakePhotoCmd();

  void getDataFromCamera();
  void zeroPadHex(byte value);  
  unsigned int getSizeFromCamera();
  
  int dumpRxToTerminal(unsigned long initTimeoutInterval, unsigned long characterTimeout, boolean extraDebugFlag);


protected:
private:
  SoftwareSerial *camSerial;
  const byte ZERO = 0;
  const unsigned long TIMEOUT_INTERVAL = 2000;
  const unsigned long UNEXPECTED_REPLY_TIMEOUT = 200;
  const unsigned long CHARACTER_TIMEOUT = 100;
  const unsigned long SHOW_MENU_INTERVAL = 20000;
  unsigned long startWaitingTime;
  unsigned long periodicTaskTimer;
  const byte RESET_CHARACTER = 'r';
  const byte PHOTO_CHARACTER = 'p';
  const byte GET_DATA_CHARACTER = 'g';
  const byte SIZE_CHARACTER = 's';
  const byte QUOTE = 34;
  const byte DATA_PER_ROW = 16;
  const byte DELAY_AFTER_RESET_SECONDS = 5;
  const int GET_DATA_BUFFER_SIZE = 0X18;
  boolean resetFlag = 0;
  boolean photoTakenFlag = 0;
};

#endif
