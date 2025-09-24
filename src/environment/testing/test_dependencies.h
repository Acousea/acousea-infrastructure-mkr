#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include <libraries.h>

#if __has_include("environment/credentials.hpp")
#include "environment/credentials.hpp"
#else
#error "No credentials file found! Please provide environment/credentials.hpp. Find an example at environment/credentials.example.hpp"
#endif

#ifdef ARDUINO
extern Uart softwareSerialSercom1;
extern Uart softwareSerialSercom0;
#endif

#define ConsoleSerial SerialUSB
// #define ConsoleSerial softwareSerialSercom0
// #define ConsoleSerial softwareSerialSercom1

// =======================================================
//       COMMON
// =======================================================
// ---- Batería ----
extern MockBatteryController mockBatteryController;
extern IBatteryController* batteryController;

// ---- GPS ----
extern IGPS* gps;
// ---- Display ----
extern IDisplay* display;

// ---- Ports -----
extern std::string TEST_HOST;

#ifdef PLATFORM_HAS_GSM
extern GsmMQTTPort gsmPort;
#endif

#ifdef PLATFORM_HAS_LORA
extern LoraPort realLoraPort;
#endif

// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef ARDUINO

// USB Display
extern SerialArduinoDisplay serialUartDisplay;

// =======================================================
//       NATIVE BUILD
// =======================================================
#else // NATIVE


#endif // ARDUINO vs NATIVE

#endif // DEPENDENCIES_H
