#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include <libraries.h>

PMICBatteryController pmicBatteryController;
AdafruitLCBatteryController adafruitLCBatteryController;
IBatteryController *battery = &adafruitLCBatteryController;

// Instancia de la pantalla
AdafruitDisplay adafruitDisplay;
SerialUSBDisplay serialUSBDisplay;
IDisplay *display = &serialUSBDisplay;

// Instancias de puertos de comunicación
SerialPort serialPort(&Serial1, 9600); // Serial1 is the hardware serial port PINS 13 and 14
LoraPort loraPort;
IridiumPort iridiumPort;
//MockLoRaPort mockLoraPort;
//MockIridiumPort mockIridiumPort;

// Instancias del GPS
MockGPS mockGPS(0.0, 0.0, 1.0);
MKRGPS mkrGPS;
UBloxGNSS uBloxGPS;
IGPS *gps = &mkrGPS;

// Instancia del controlador de tiempo real
RTCController rtcController;

SDManager sdManager;
NodeConfigurationRepository nodeConfigurationRepository(sdManager, "config.txt");

// Instancias de rutinas de servicio
SetNodeConfigurationRoutine setNodeConfigurationRoutine(nodeConfigurationRepository);
CompleteSummaryReportRoutine completeSummaryReportRoutine(nodeConfigurationRepository);
BasicSummaryReportRoutine basicSummaryReportRoutine(gps, battery, &rtcController, nodeConfigurationRepository);

std::map<OperationCode::Code, IRoutine<Packet> *> configurationRoutines = {
        {OperationCode::Code::SET_NODE_DEVICE_CONFIG, &setNodeConfigurationRoutine},
};

std::map<OperationCode::Code, IRoutine<VoidType> *> reportingRoutines = {
        {OperationCode::Code::BASIC_STATUS_REPORT,    &basicSummaryReportRoutine},
        {OperationCode::Code::COMPLETE_STATUS_REPORT, &completeSummaryReportRoutine},
};


// Instancia del procesador de paquetes
PacketProcessor packetProcessor(configurationRoutines);


// Instancia del router
Router router = Router(
        {&serialPort, &loraPort, &iridiumPort}
);

// Instancia del runner
NodeOperationRunner nodeOperationRunner(display,
                                        router,
                                        packetProcessor,
                                        reportingRoutines,
                                        nodeConfigurationRepository
);

#endif // DEPENDENCIES_H