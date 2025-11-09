#ifndef ACOUSEA_LIBRARIES_H
#define ACOUSEA_LIBRARIES_H

// ================== COMMON TO ALL SUPPORTED PLATFORMS ==================
#if defined(PLATFORM_ARDUINO) || defined(PLATFORM_NATIVE)
// ------------------------- PROTOBUF -------------------------
#include <bindings/nodeDevice.pb.h>

// ------------------------- SHARED LIBRARIES -------------------------
#include <Result.h>
#include "Logger/Logger.h"
#include <ErrorHandler/ErrorHandler.h>
#include <ClassName.h>
#include "time/getMillis.hpp"
#include "WatchDog/WatchDogUtils.hpp"

// ------------------ BATTERY ------------------
#include <IBatteryController.h>

// ------------------ GPS ------------------
#include <IGPS.h>

// ------------------ ROUTER ------------------
#include <Router.h>

// ------------------ DISPLAY ------------------
#include <IDisplay.h>

// ------------------ STORAGE ------------------
#include <StorageManager/StorageManager.hpp>

// ------------------ PORTS ------------------
#include <Ports/IPort.h>

// ------------------ RTC ------------------
#include "RTCController.hpp"

// ------------------------- ROUTINES -------------------------
#include <routines/SetNodeConfigurationRoutine/SetNodeConfigurationRoutine.h>
#include <routines/CompleteStatusReportRoutine/CompleteStatusReportRoutine.h>
#include <routines/GetUpdatedNodeConfigurationRoutine/GetUpdatedNodeConfigurationRoutine.hpp>
#include "routines/StoreNodeConfigurationRoutine/StoreNodeConfigurationRoutine.h"
#include "routines/RelayPacketRoutine/RelayPacketRoutine.hpp"

// ------------------------- REPOSITORIES -------------------------
#include <NodeConfigurationRepository/NodeConfigurationRepository.h>

// ------------------------- DRIVERS -------------------------
#include <NodeOperationRunner/NodeOperationRunner.h>
#include <ModuleProxy/ModuleProxy.hpp>
#include <TaskScheduler/TaskScheduler.h>
#include <TaskScheduler/FunctionTask.hpp>
#include <TaskScheduler/LambdaTask.hpp>
#include <TaskScheduler/MethodTask.hpp>

#endif

// ================== DEPEDNING ON PLATFORM ==================
#if defined(PLATFORM_ARDUINO)
#include <Arduino.h>
#include "wiring_private.h"
// ------------------ STORAGE ------------------
#include <StorageManager/SDStorageManager/SDStorageManager.h>

// ------------------ BATTERY ------------------
#include <PMICBatteryController/PMICBatteryController.h>
#include <AdafruitLCBatteryController/AdafruitLCBatteryController.h>
#include <MockBatteryController/MockBatteryController.h>
#include <SolarXBatteryController/SolarXBatteryController.h>

// ------------------ GPS ------------------
#include <MockGPS/MockGPS.h>
#include <MKRGPS/MKRGPS.h>
#include <UBloxGPS/UBloxGPS.h>

// ------------------ RTC ------------------
#include "ZeroRTCController/ZeroRTCController.h"
#include "MockRTCController/MockRTCController.h"

// ------------------ DISPLAY ------------------
#include <AdafruitDisplay/AdafruitDisplay.h>
#include <SerialArduinoDisplay/SerialArduinoDisplay.h>

// ------------------ PORTS ------------------
#include <Ports/Serial/SerialPort.h>
#include <Ports/LoRa/LoRaPort.h>
#include <Ports/Iridium/IridiumPort.h>
#include <Ports/Serial/MockSerialPort.h>
#include <Ports/LoRa/MockLoRaPort.h>
#include <Ports/Iridium/MockIridiumPort.h>
#include <Ports/Serial/MockSerialPort.h>
#include <Ports/GSM/GsmMQTTPort.hpp>

// ------------------ DRIVERS  ------------------
#include <MosfetController/MosfetController.hpp>
#include <PiController/PiController.hpp>

// ------------------ POLICIES  ------------------
#include "BatteryProtectionPolicy/BatteryProtectionPolicy.hpp"



#elif defined(PLATFORM_NATIVE)

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
#include <Ports/Serial/MockSerialPort.h>
#include <Ports/LoRa/MockLoRaPort.h>
#include <Ports/Iridium/MockIridiumPort.h>
#include <Ports/http/HttpPort.hpp>
#include <Ports/Serial/NativeSerialPort.h>


#else // NATIVE
#error "No valid PLATFORM defined. Please define PLATFORM as PLATFORM_ARDUINO or PLATFORM_NATIVE."
#endif


#endif

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
