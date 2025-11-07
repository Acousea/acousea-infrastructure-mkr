#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

// #undef PLATFORM_ARDUINO
// #define PLATFORM_NATIVE

#include "../shared_utils.hpp"
#include <libraries.h>



#if __has_include("environment/credentials.hpp")
#include "environment/credentials.hpp"
#else
#error "No credentials file found! Please provide environment/credentials.hpp. Find an example at environment/credentials.example.hpp"
#endif


#if defined(PLATFORM_ARDUINO)
extern Uart softwareSerialSercom1;
extern Uart softwareSerialSercom0;
#define ConsoleSerial SerialUSB
// #define ConsoleSerial softwareSerialSercom0
// #define ConsoleSerial softwareSerialSercom1

// ---- Batería ----
extern MockBatteryController mockBatteryController;
extern SolarXBatteryController solarXBatteryController;
extern IBatteryController& batteryControllerRef;

// ---- GPS ----
extern IGPS& gpsRef;

// ---- Display ----
extern IDisplay& displayRef;

// ---- Ports -----
#ifdef PLATFORM_HAS_GSM
extern GsmMQTTPort gsmPort;
#endif

#ifdef PLATFORM_HAS_LORA
extern LoraPort realLoraPort;
#endif

extern TaskScheduler scheduler;

// USB Display
extern SerialArduinoDisplay serialUartDisplay;

extern PiController piPowerController;

extern StorageManager& storageManagerRef;

extern RTCController& rtcControllerRef;

extern BatteryProtectionPolicy batteryProtectionPolicy;

// =======================================================
//       NATIVE BUILD
// =======================================================
#elif defined(PLATFORM_NATIVE)

static const NativeConsole consoleNative;
#define ConsoleSerial consoleNative

// ---- Batería ----
extern IBatteryController& batteryControllerRef;

// ---- GPS ----
extern IGPS* gps;

// ---- Display ----
extern IDisplay& displayRef;


// ---- Task Scheduler ----
extern TaskScheduler scheduler;

// ----- Storage Manager -----
extern StorageManager& storageManagerRef;

// ----- RTC Controller -----
extern RTCController& rtcControllerRef;


#else // NATIVE
#error "No valid PLATFORM defined. Please define PLATFORM as PLATFORM_ARDUINO or PLATFORM_NATIVE."

#endif // PLATFORM_ARDUINO


#endif // DEPENDENCIES_H
