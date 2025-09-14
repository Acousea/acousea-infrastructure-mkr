#ifndef libraries_h
#define libraries_h

// ------------------ ARDUINO LIBRARIES ------------------
//#include <Arduino.h>
//#include "wiring_private.h"
//#include <SPI.h>
//#include <Wire.h>
//#include <SD.h>
//#include <LoRa.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//#include <Adafruit_LC709203F.h>
//#include <Arduino_MKRGPS.h>
//#include <Arduino_PMIC.h>
//#include <RTCZero.h>
//#include <IridiumSBD.h>
//#include <ArduinoJson.h>
//#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
//#include <ArduinoLowPower.h>
//#include <Adafruit_SleepyDog.h>

// ================== Common ==================
#include <Router.h>
#include <StorageManager/StorageManager.hpp>
#include <bindings/nodeDevice.pb.h>

// ------------------------- SHARED LIBRARIES -------------------------
#include <Result/Result.h>
#include <CRC/CRCUtils.h>
#include <StorageManager/StorageManager.hpp>
#include "Logger/Logger.h"
#include <ErrorHandler/ErrorHandler.h>
#include <ClassName.h>

// ------------------ DISPLAY ------------------
#include <IDisplay.h>

// ------------------ STORAGE ------------------
#include <StorageManager/StorageManager.hpp>

// ------------------ PORTS ------------------
#include <Ports/IPort.h>

// ------------------------- ROUTINES -------------------------
#include <routines/SetNodeConfigurationRoutine/SetNodeConfigurationRoutine.h>
#include <routines/CompleteStatusReportRoutine/CompleteStatusReportRoutine.h>
#include <routines/BasicStatusReportRoutine/BasicStatusReportRoutine.h>
#include <routines/GetUpdatedNodeConfigurationRoutine/GetUpdatedNodeConfigurationRoutine.hpp>
#include "routines/StoreNodeConfigurationRoutine/StoreNodeConfigurationRoutine.h"

// ------------------------- REPOSITORIES -------------------------
#include <NodeConfigurationRepository/NodeConfigurationRepository.h>

// ------------------------- SERVICES -------------------------
#include <NodeOperationRunner/NodeOperationRunner.h>
#include <ICListenService/ICListenService.h>


// ================== DEPEDNING ON PLATFORM ==================
#ifdef ARDUINO
#include <Arduino.h>

// ------------------ STORAGE ------------------
#include <StorageManager/SDStorageManager/SDStorageManager.h>

// ------------------ BATTERY ------------------
#include <PMICBatteryController/PMICBatteryController.h>
#include <AdafruitLCBatteryController/AdafruitLCBatteryController.h>
#include <MockBatteryController/MockBatteryController.h>

// ------------------ GPS ------------------
#include <MockGPS/MockGPS.h>
#include <MKRGPS/MKRGPS.h>
#include <UBloxGPS/UBloxGPS.h>

// ------------------ RTC ------------------
#include "ZeroRTCController/ZeroRTCController.h"
#include "MockRTCController/MockRTCController.h"

// ------------------ DISPLAY ------------------
#include <AdafruitDisplay/AdafruitDisplay.h>
#include <SerialUSBDisplay/SerialUSBDisplay.h>

// ------------------ PORTS ------------------
#include <Ports/Serial/SerialPort.h>
#include <Ports/LoRa/LoRaPort.h>
#include <Ports/Iridium/IridiumPort.h>
#include <Ports/Serial/MockSerialPort.h>
#include <Ports/LoRa/MockLoRaPort.h>
#include <Ports/Iridium/MockIridiumPort.h>


// ------------------ CONTROLLERS ------------------
#include <MosfetController/MosfetController.hpp>
#include <RockPiPowerController/RockPiPowerController.hpp>

#else // NATIVE

// ------------------ STORAGE ------------------
#include <StorageManager/HDDStorageManager/HddStorageManager.hpp>

// ------------------ BATTERY ------------------
#include <MockBatteryController/MockBatteryController.h>

// ------------------ GPS ------------------
#include <MockGPS/MockGPS.h>

// ------------------ RTC ------------------
#include "MockRTCController/MockRTCController.h"

// ------------------ DISPLAY ------------------
#include <ConsoleDisplay/ConsoleDisplay.hpp>

// ------------------ PORTS ------------------
#include "Ports/Serial/NativeSerialPort.h"
#include <Ports/Serial/MockSerialPort.h>
#include <Ports/LoRa/MockLoRaPort.h>
#include <Ports/Iridium/MockIridiumPort.h>
#include <Ports/http/HttpPort.hpp>



#endif



#endif