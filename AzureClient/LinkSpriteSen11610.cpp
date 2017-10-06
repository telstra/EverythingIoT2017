#include "LinkSpriteSen11610.h"

void LinkSprite::SendResetCmd()
{
    camSerial->write(0x56);
    camSerial->write(ZERO);
    camSerial->write(0x26);
    camSerial->write(ZERO);
}

void LinkSprite::SetImageSizeCmd(byte Size)
{
    camSerial->write(0x56);
    camSerial->write(ZERO);
    camSerial->write(0x54);
    camSerial->write(0x01);
    camSerial->write(Size);
}

void LinkSprite::SetBaudRateCmd(byte baudrate)
{
    camSerial->write(0x56);
    camSerial->write(ZERO);
    camSerial->write(0x24);
    camSerial->write(0x03);
    camSerial->write(0x01);
    camSerial->write(baudrate);
}

//Send take picture command
void LinkSprite::SendTakePhotoCmd()
{
    camSerial->write(0x56);
    camSerial->write(ZERO);
    camSerial->write(0x36);
    camSerial->write(0x01);
    camSerial->write(ZERO);
}

//Read data
int LinkSprite::SendReadDataCmd(int cameraAddress)
{
    camSerial->write(0x56);
    camSerial->write(ZERO);
    camSerial->write(0x32);
    camSerial->write(0x0c);
    camSerial->write(ZERO);
    camSerial->write(0x0a);
    camSerial->write(ZERO);
    camSerial->write(ZERO);
    camSerial->write(highByte(cameraAddress));
    camSerial->write(lowByte(cameraAddress));
    camSerial->write(ZERO);
    camSerial->write(ZERO);
    camSerial->write(ZERO);
    camSerial->write(GET_DATA_BUFFER_SIZE);
    camSerial->write(ZERO);
    camSerial->write(0x0a);
    cameraAddress += GET_DATA_BUFFER_SIZE; //address increases 32£¬set according to buffer size
    return cameraAddress;
}

void LinkSprite::StopTakePhotoCmd()
{
    camSerial->write(0x56);
    camSerial->write(ZERO);
    camSerial->write(0x36);
    camSerial->write(0x01);
    camSerial->write(0x03);
}

void LinkSprite::getDataFromCamera()
// I think this function has problems.
// There presently a timeout so the
// function will continue to loop if
// the end data characters are not
// read from the camera.
{
    byte rxBuffer[GET_DATA_BUFFER_SIZE];
    boolean endFlag = 0;
    int cameraAddress = 0;

    while (!endFlag)
    {
        int rxBufferIndex = 0;
        int indexK = 0;
        int countIndex = 0;
        cameraAddress = SendReadDataCmd(cameraAddress);

        delay(25);
        while (camSerial->available() > 0)
        {
            byte inputCharacter = camSerial->read();
            indexK++;
            if ((indexK > 5) && (rxBufferIndex < GET_DATA_BUFFER_SIZE) && (!endFlag))
            {
                rxBuffer[rxBufferIndex] = inputCharacter;
                if ((rxBuffer[rxBufferIndex - 1] == 0xFF) && (rxBuffer[rxBufferIndex] == 0xD9)) //Check if the picture is over
                    endFlag = 1;
                rxBufferIndex++;
                countIndex++;
            }
        }

        for (int i = 0; i < countIndex; i++)
        {
            if (rxBuffer[i] < 0x10)
            {
                Serial.print("0");
            }
            Serial.print(rxBuffer[i], HEX);
            Serial.print(" ");
        } //Send jpeg picture over the serial port
        Serial.println();
    }
}

void LinkSprite::zeroPadHex(byte value)
{
    if (value < 0x10)
    {
        Serial.print("0");
    }
    Serial.print(value, HEX);
}

unsigned int LinkSprite::getSizeFromCamera()
{
    char result[9];
    int charCount = 0;
    int length = 0;

    uint16_t test = 27560;

    result[7] = highByte(test);
    Serial.println((unsigned int)result[7]);
    result[8] = lowByte(test);
    Serial.println((unsigned int)result[8]);

    camSerial->write(0x56);
    camSerial->write(ZERO);
    camSerial->write(0x34);
    camSerial->write(0x01);
    camSerial->write(ZERO);

    delay(1000); // wait for the command to execute on the camera

//    while (camSerial->available() > 0 && charCount < 9) // expect 9 bytes back from camera
//    {
//        result[charCount] = camSerial->read();
//        charCount++;
//    }

    length = result[7];
    length = length << 8;
    length += result[8];

    Serial.print("length ");
    Serial.println(length);

    return length; 
}

int LinkSprite::dumpRxToTerminal(unsigned long initTimeoutInterval, unsigned long characterTimeout, boolean extraDebugFlag)
{
    unsigned long beginTimeout = millis();
    unsigned long lastCharacter = millis();
    boolean noRxYetFlag = 1;
    int receivedCharacters = 0;
    while (((millis() - beginTimeout < initTimeoutInterval) && noRxYetFlag) || (millis() - lastCharacter < characterTimeout))
    {
        if (camSerial->available() > 0)
        {
            if (noRxYetFlag)
            {
                noRxYetFlag = 0;
                Serial.print("received from camera = ");
            }
            byte inputCharacter = camSerial->read();
            zeroPadHex(inputCharacter);
            Serial.print(" ");
            lastCharacter = millis(); // reset lastCharacter timer
            receivedCharacters++;
            if (receivedCharacters % DATA_PER_ROW == 0)
            {
                Serial.println();
            }
        }
    }
    if (noRxYetFlag && extraDebugFlag)
    {
        Serial.println("Timeout with no data from camera.");
    }
    else if (extraDebugFlag)
    {
        Serial.println();
        Serial.println("End of received data.");
    }
    return receivedCharacters;
}
