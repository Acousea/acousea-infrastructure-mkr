#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include <libraries.h>

extern PMICBatteryController pmicBatteryController;
extern AdafruitLCBatteryController adafruitLCBatteryController;
extern MockBatteryController mockBatteryController;
extern IBatteryController *battery;

extern AdafruitDisplay adafruitDisplay;
extern SerialUSBDisplay serialUSBDisplay;
extern IDisplay *display;

extern SerialPort serialPort;
extern LoraPort realLoraPort;
extern IridiumPort realIridiumPort;
extern MockLoRaPort mockLoraPort;
extern MockIridiumPort mockIridiumPort;

extern MockGPS mockGPS;
extern MKRGPS mkrGPS;
extern UBloxGNSS uBloxGPS;
extern IGPS *gps;

extern ZeroRTCController zeroRTCController;
extern SDManager sdManager;
extern Router router;
extern NodeConfigurationRepository nodeConfigurationRepository;
extern ICListenService icListenService;

extern SetNodeConfigurationRoutine setNodeConfigurationRoutine;
extern CompleteStatusReportRoutine completeSummaryReportRoutine;
extern BasicStatusReportRoutine basicSummaryReportRoutine;
extern StoreICListenConfigurationRoutine storeICListenConfigurationRoutine;

extern std::map<OperationCode::Code, IRoutine<Packet> *> configurationRoutines;
extern std::map<OperationCode::Code, IRoutine<VoidType> *> reportingRoutines;

extern NodeOperationRunner nodeOperationRunner;

extern MosfetController mosfetController;
extern RockPiPowerController rockPiPowerController;

#endif // DEPENDENCIES_H