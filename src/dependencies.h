#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include <libraries.h>

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
extern IPort* loraPort;
extern IPort* iridiumPort;

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
extern CompleteStatusReportRoutine completeSummaryReportRoutine;
extern BasicStatusReportRoutine basicSummaryReportRoutine;


extern std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> configurationRoutines;

extern NodeOperationRunner nodeOperationRunner;


// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef ARDUINO

// USB Display
extern SerialUSBDisplay serialUSBDisplay;

// RealLoraPort
extern LoraPort realLoraPort;

// Power
extern MosfetController mosfetController;
extern RockPiPowerController rockPiPowerController;

// =======================================================
//       NATIVE BUILD
// =======================================================
#else // NATIVE


#endif // ARDUINO vs NATIVE

#endif // DEPENDENCIES_H


