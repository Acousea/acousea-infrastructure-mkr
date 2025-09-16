#include "dev_dependencies.h"

#include "Ports/Serial/MockSerialPort.h"
#include "SolarXBatteryController/SolarXBatteryController.h"


// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef ARDUINO
// --------- Batería ----------
PMICBatteryController pmicBatteryController;
AdafruitLCBatteryController adafruitLCBatteryController;
MockBatteryController mockBatteryController;
SolarXBatteryController solarXBatteryController(std::vector<uint8_t>{0x40, 0x41});
IBatteryController* batteryController = &solarXBatteryController; // o PMIC según HW

// --------- Display ----------
AdafruitDisplay adafruitDisplay;
SerialUSBDisplay serialUSBDisplay;
IDisplay* display = &serialUSBDisplay;

// --------- Puertos ----------
SerialPort realSerialPort(&Serial1, 4800);
MockLoRaPort mockLoraPort;
MockIridiumPort mockIridiumPort;
// LoraPort realLoraPort;
// IridiumPort realIridiumPort;

IPort* serialPort = &realSerialPort;
IPort* loraPort = &mockLoraPort;
IPort* iridiumPort = &mockIridiumPort;

// --------- GPS ----------
MockGPS mockGPS(0.0, 0.0, 1.0);
MKRGPS mkrGPS;
UBloxGNSS uBloxGPS;
IGPS* gps = &mockGPS;

// --------- RTC ----------
ZeroRTCController zeroRTCController;
MockRTCController mockRTCController;
RTCController* rtcController = &mockRTCController;

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
IBatteryController* batteryController = &mockBatteryController;

// --------- Display ----------
ConsoleDisplay consoleDisplay;
IDisplay* display = &consoleDisplay;

// --------- Puertos ----------
MockSerialPort mockSerialPort;
NativeSerialPort nativeSerialPort("/tmp/ttyV0", 9600);
MockLoRaPort mockLoraPort;
MockIridiumPort mockIridiumPort;
HttpPort httpPort("http://127.0.0.1:8000", "123456789012345");

IPort* serialPort = &nativeSerialPort;
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
                                                                      batteryController,
                                                                      rtcController
);

// GetUpdatedNodeConfigurationRoutine getUpdatedNodeConfigurationRoutine(nodeConfigurationRepository,
//                                                                       std::nullopt,
//                                                                       gps,
//                                                                       battery,
//                                                                       rtcController
// );

CompleteStatusReportRoutine completeStatusReportRoutine(nodeConfigurationRepository,
                                                        icListenServicePtr,
                                                        gps,
                                                        batteryController,
                                                        rtcController
);

// CompleteStatusReportRoutine completeSummaryReportRoutine(nodeConfigurationRepository,
//                                                          std::nullopt,
//                                                          gps,
//                                                          batteryController,
//                                                          rtcController
// );s

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
    {acousea_ReportBody_statusPayload_tag, &completeStatusReportRoutine},
};


NodeOperationRunner nodeOperationRunner(
    router,
    nodeConfigurationRepository,
    commandRoutines, responseRoutines, reportRoutines
);
