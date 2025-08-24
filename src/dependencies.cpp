#include "dependencies.h"


// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef ARDUINO
// --------- Batería ----------
PMICBatteryController pmicBatteryController;
AdafruitLCBatteryController adafruitLCBatteryController;
MockBatteryController mockBatteryController;
IBatteryController* battery = &adafruitLCBatteryController; // o PMIC según HW

// --------- Display ----------
AdafruitDisplay adafruitDisplay;
SerialUSBDisplay serialUSBDisplay;
IDisplay* display = &adafruitDisplay;

// --------- Puertos ----------
SerialPort realSerialPort(&Serial1, 4800);
LoraPort realLoraPort;
IridiumPort realIridiumPort;

IPort* serialPort = &realSerialPort;
IPort* loraPort = &realLoraPort;
IPort* iridiumPort = &realIridiumPort;

// --------- GPS ----------
MockGPS mockGPS(0.0, 0.0, 1.0);
MKRGPS mkrGPS;
UBloxGNSS uBloxGPS;
IGPS* gps = &mkrGPS;

// --------- RTC ----------
ZeroRTCController zeroRTCController;
MockRTCController mockRTCController;
RTCController* rtcController = &zeroRTCController;

// --------- Storage ----------
SDStorageManager sdStorageManager;
StorageManager* storageManager = &sdStorageManager; // o hddStorageManager según build

// --------- Router ----------
Router router({serialPort, loraPort, iridiumPort});

// --------- Power ----------
MosfetController mosfetController;
RockPiPowerController rockPiPowerController(mosfetController);


// =======================================================
//       NATIVE BUILD
// =======================================================
#else // NATIVE

// --------- Batería ----------
MockBatteryController mockBatteryController;
IBatteryController* battery = &mockBatteryController;

// --------- Display ----------
ConsoleDisplay consoleDisplay;
IDisplay* display = &consoleDisplay;

// --------- Puertos ----------
MockSerialPort mockSerialPort;
MockLoRaPort mockLoraPort;
MockIridiumPort mockIridiumPort;
HttpPort httpPort("http://localhost:8080", "http://localhost:8080", 10000);

IPort* serialPort = &mockSerialPort;
IPort* loraPort = &mockLoraPort;
IPort* iridiumPort = &httpPort;


// --------- GPS ----------
MockGPS mockGPS(0.0,0.0,1.0);
IGPS* gps = &mockGPS;

// --------- RTC ----------
MockRTCController mockRTCController;
RTCController* rtcController = &mockRTCController;

// --------- Storage ----------
HDDStorageManager hddStorageManager;
StorageManager* storageManager = &hddStorageManager; // o hddStorageManager según build

// --------- Router ----------
Router router({ serialPort, loraPort, iridiumPort });


#endif // ARDUINO vs NATIVE

// =======================================================
//       COMÚN A AMBOS
// =======================================================


NodeConfigurationRepository nodeConfigurationRepository(*storageManager, "config.txt");
ICListenService icListenService(router);

SetNodeConfigurationRoutine setNodeConfigurationRoutine(nodeConfigurationRepository);
CompleteStatusReportRoutine completeSummaryReportRoutine(gps, battery, nodeConfigurationRepository, icListenService);
BasicStatusReportRoutine basicSummaryReportRoutine(gps, battery, rtcController, nodeConfigurationRepository);

std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> configurationRoutines = {
    {acousea_PayloadWrapper_setConfiguration_tag, &setNodeConfigurationRoutine},
};

// FIXME: RequestedConfiguration should not execute the basicSummaryReportRoutine
std::map<uint8_t, IRoutine<VoidType>*> reportingRoutines = {
    {acousea_PayloadWrapper_requestedConfiguration_tag, &basicSummaryReportRoutine},
    {acousea_PayloadWrapper_statusPayload_tag, &completeSummaryReportRoutine},
};

NodeOperationRunner nodeOperationRunner(
    router,
    reportingRoutines,
    configurationRoutines,
    nodeConfigurationRepository
);
