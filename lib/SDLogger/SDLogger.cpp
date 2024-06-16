#include "SDLogger.h"



bool SDLogger::init()
{
    SerialUSB.println("Initializing SD card...");
    LoRa.setPins(18, 14, 26);
    return true;
}