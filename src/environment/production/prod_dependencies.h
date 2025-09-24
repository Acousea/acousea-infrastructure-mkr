#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include <libraries.h>
#if __has_include("environment/credentials.hpp")
  #include "environment/credentials.hpp"
#else
  #error "No credentials file found! Please provide environment/credentials.hpp. Find an example at environment/credentials.example.hpp"
#endif


// =======================================================
//       COMMON
// =======================================================
// ---- Batería ----
extern MockBatteryController mockBatteryController;
extern IBatteryController* batteryController;

// ---- Display ----
extern IDisplay* display;

// --- Puertos -----
extern IPort* serialPort;
extern IPort* iridiumPort;

#if defined(PLATFORM_HAS_LORA) || defined(PLATFORM_HAS_GSM)
extern IPort* loraOrGsmPort;
#endif


// ---- GPS ----
extern IGPS* gps;

// ---- RTC ----
extern RTCController* rtcController;

// ---- Router ----
extern Router router;

// ---- Storage ----
extern StorageManager* storageManager;

// Repo + Services
extern NodeConfigurationRepository nodeConfigurationRepository;

// IcListen
extern std::shared_ptr<ICListenService> icListenServicePtr;

// Routines
extern SetNodeConfigurationRoutine setNodeConfigurationRoutine;
extern CompleteStatusReportRoutine completeStatusReportRoutine;

extern std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> configurationRoutines;

extern NodeOperationRunner nodeOperationRunner;


// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef ARDUINO
extern Uart softwareSerialSercom1;
extern Uart softwareSerialSercom0;

// USB Display
#define ConsoleSerial SerialUSB
// #define ConsoleSerial softwareSerialSercom0
// #define ConsoleSerial softwareSerialSercom1
extern SerialArduinoDisplay serialUSBDisplay;

#ifdef PLATFORM_HAS_GSM
extern GsmMQTTPort gsmPort;
#endif
// RealLoraPort
#ifdef PLATFORM_HAS_LORA
extern LoraPort realLoraPort;
#endif


// Power
extern MosfetController mosfetController;
extern RockPiPowerController rockPiPowerController;

// =======================================================
//       NATIVE BUILD
// =======================================================
#else // NATIVE


#endif // ARDUINO vs NATIVE

#endif // DEPENDENCIES_H
