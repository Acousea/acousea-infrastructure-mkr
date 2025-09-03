#include "dependencies.h"

#include "routines/StoreNodeConfigurationRoutine/StoreNodeConfigurationRoutine.h"


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
HttpPort httpPort("http://10.22.146.50:8000", "123456789012345");

IPort* serialPort = &mockSerialPort;
IPort* loraPort = &mockLoraPort;
IPort* iridiumPort = &httpPort;


// --------- GPS ----------
MockGPS mockGPS(0.0, 0.0, 1.0);
IGPS* gps = &mockGPS;

// --------- RTC ----------
MockRTCController mockRTCController;
RTCController* rtcController = &mockRTCController;

// --------- Storage ----------
HDDStorageManager hddStorageManager;
StorageManager* storageManager = &hddStorageManager; // o hddStorageManager según build

// --------- Router ----------
Router router({serialPort, loraPort, iridiumPort});


#endif // ARDUINO vs NATIVE

// =======================================================
//       COMÚN A AMBOS
// =======================================================


NodeConfigurationRepository nodeConfigurationRepository(*storageManager);

std::shared_ptr<ICListenService> icListenServicePtr = std::make_shared<ICListenService>(router);

SetNodeConfigurationRoutine setNodeConfigurationRoutine(nodeConfigurationRepository, icListenServicePtr);
// SetNodeConfigurationRoutine setNodeConfigurationRoutine(nodeConfigurationRepository, std::nullopt);

GetUpdatedNodeConfigurationRoutine getUpdatedNodeConfigurationRoutine(nodeConfigurationRepository,
                                                                      icListenServicePtr,
                                                                      gps,
                                                                      battery,
                                                                      rtcController
);

// GetUpdatedNodeConfigurationRoutine getUpdatedNodeConfigurationRoutine(nodeConfigurationRepository,
//                                                                       std::nullopt,
//                                                                       gps,
//                                                                       battery,
//                                                                       rtcController
// );

CompleteStatusReportRoutine completeSummaryReportRoutine(nodeConfigurationRepository,
                                                         icListenServicePtr,
                                                         gps,
                                                         battery
);

// CompleteStatusReportRoutine completeSummaryReportRoutine(nodeConfigurationRepository,
//                                                          std::nullopt,
//                                                          gps,
//                                                          battery
// );

BasicStatusReportRoutine basicSummaryReportRoutine(
    nodeConfigurationRepository, gps, battery, rtcController
);

StoreNodeConfigurationRoutine storeNodeConfigurationRoutine(
    nodeConfigurationRepository,
    icListenServicePtr
);

std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> commandRoutines = {
    {acousea_CommandBody_setConfiguration_tag, &setNodeConfigurationRoutine},
    {acousea_CommandBody_requestedConfiguration_tag, &getUpdatedNodeConfigurationRoutine},
};

std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> responseRoutines = {
    {acousea_ResponseBody_setConfiguration_tag, &storeNodeConfigurationRoutine},
    {acousea_ResponseBody_updatedConfiguration_tag, &storeNodeConfigurationRoutine},
};


std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> reportRoutines = {
    {acousea_ReportBody_statusPayload_tag, &completeSummaryReportRoutine},
    {acousea_ReportBody_statusPayload_tag, &basicSummaryReportRoutine},
};


NodeOperationRunner nodeOperationRunner(
    router,
    nodeConfigurationRepository,
    commandRoutines, responseRoutines, reportRoutines
);
