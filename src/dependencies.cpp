#include "dependencies.h"

PMICBatteryController pmicBatteryController;
AdafruitLCBatteryController adafruitLCBatteryController;
MockBatteryController mockBatteryController;
IBatteryController *battery = &mockBatteryController;

AdafruitDisplay adafruitDisplay;
SerialUSBDisplay serialUSBDisplay;
IDisplay *display = &serialUSBDisplay;

SerialPort serialPort(&Serial1, 4800);
LoraPort realLoraPort;
IridiumPort realIridiumPort;
MockLoRaPort mockLoraPort;
MockIridiumPort mockIridiumPort;

MockGPS mockGPS(0.0, 0.0, 1.0);
MKRGPS mkrGPS;
UBloxGNSS uBloxGPS;
IGPS *gps = &mockGPS;

RTCController rtcController;
SDManager sdManager;
Router router({&serialPort, &mockLoraPort, &mockIridiumPort});

NodeConfigurationRepository nodeConfigurationRepository(sdManager, "config.txt");
ICListenService icListenService(router);

SetNodeConfigurationRoutine setNodeConfigurationRoutine(nodeConfigurationRepository);
CompleteStatusReportRoutine completeSummaryReportRoutine(gps, battery, nodeConfigurationRepository, icListenService);
BasicStatusReportRoutine basicSummaryReportRoutine(gps, battery, &rtcController, nodeConfigurationRepository);
StoreICListenConfigurationRoutine storeICListenConfigurationRoutine(icListenService);

std::map<OperationCode::Code, IRoutine<Packet> *> configurationRoutines = {
    {OperationCode::Code::SET_NODE_DEVICE_CONFIG, &setNodeConfigurationRoutine},
    {OperationCode::Code::SET_ICLISTEN_CONFIG, &storeICListenConfigurationRoutine},
};

std::map<OperationCode::Code, IRoutine<VoidType> *> reportingRoutines = {
    {OperationCode::Code::BASIC_STATUS_REPORT, &basicSummaryReportRoutine},
    {OperationCode::Code::COMPLETE_STATUS_REPORT, &completeSummaryReportRoutine},
};

NodeOperationRunner nodeOperationRunner(router, reportingRoutines, configurationRoutines, nodeConfigurationRepository);