#include <Arduino.h>
// #include "../../include/config.h"
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino_MKRGPS.h>
#include <Arduino_PMIC.h>
#include <RTCZero.h>
#include <IridiumSBD.h> 

class SDLogger
{
public:
    bool init();    
};