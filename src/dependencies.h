
#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include <libraries.h>
#include <config.h>
#include <map>
#include "../lib/Port/IPort.h"
#include "../lib/Port/LoRaPort.h"
#include "../lib/Port/MockLoRaPort.h"
#include "../lib/Port/IridiumPort.h"
#include "../lib/Port/MockIridiumPort.h"
#include "../lib/Port/SerialPort.h"
#include "../lib/Router/Router.h"
#include "../lib/Processor/PacketProcessor.h"

#include "../lib/Display/AdafruitDisplay.h"
#include "../lib/Display/SerialUSBDisplay.h"

#include "../lib/Routines/PingRoutine.h"
#include "../lib/Routines/ChangeOpModeRoutine.h"
#include "../lib/Routines/SummaryRoutine.h"
#include "../lib/Routines/ReportRoutine.h"
#include "../lib/Routines/SimpleReportRoutine.h"
#include "../lib/RoutingTable/RoutingTable.h"

#include "../lib/Battery/PMICBatteryController.h"
#include "../lib/Battery/AdafruitLCBatteryController.h"

#include "../GPS/MockGPS/MockGPS.h"
#include "../GPS/MKRGPS/MKRGPS.h"
#include "../GPS/UBloxGPS/UBloxGPS.h"
#include "../lib/RTC/RTCController.h"

#include "../lib/OperationManager/OperationManager.h"

#include "../lib/Services/SummaryService.h"
#include "../lib/Services/ReportService.h"
#include "../lib/Services/SimpleReportService.h"

#include "../lib/OperationModes/Drifter/LaunchingMode.h"
#include "../lib/OperationModes/Drifter/WorkingMode.h"
#include "../lib/OperationModes/Drifter/RecoveryMode.h"

#include "../lib/OperationModes/Localizer/LaunchingMode.h"
#include "../lib/OperationModes/Localizer/WorkingMode.h"
#include "../lib/OperationModes/Localizer/RecoveryMode.h"

#include "../lib/SDManager/SDManager.h"
#include "../lib/OperationModes/ReportingPeriodManager/ReportingPeriodManager.h"

#include "../lib/Routines/SetReportingPeriodsRoutine.h"
#include "../lib/Routines/GetReportingPeriodsRoutine.h"


// FIXME: Must define OperationMode class to manage the state of the system
PMICBatteryController pmicBatteryController;
AdafruitLCBatteryController adafruitLCBatteryController;

// Instancia de la pantalla
AdafruitDisplay adafruitDisplay;
SerialUSBDisplay serialUSBDisplay;

// Instancias de puertos de comunicación
SerialPort serialPort(&Serial1, 9600); // Serial1 is the hardware serial port PINS 13 and 14
LoraPort loraPort;
MockLoRaPort mockLoraPort;
IridiumPort iridiumPort;
MockIridiumPort mockIridiumPort;

// Instancias del GPS
MockGPS mockGPS(0.0, 0.0, 1.0);
MKRGPS mkrGPS;
UBloxGNSS uBloxGPS;

// Instancia del controlador de tiempo real
RTCController rtcController(&mockGPS);

SDManager sdManager(SDCARD_SS_PIN);
ReportingPeriodManager reportingPeriodManager(sdManager, "config.txt");
// Instancia del administrador de operaciones
OperationManager operationManager;

// Services
SummaryService summaryService;
ReportService reportService;
SimpleReportService simpleReportService;

// Instancias de rutinas de servicio
PingRoutine pingRoutine;
ChangeOperationModeRoutine changeOpModeRoutine(operationManager);
GetReportingPeriodsRoutine getRepolrtingPeriodsRoutine(reportingPeriodManager);
SetReportingPeriodsRoutine setReportingPeriodsRoutine(reportingPeriodManager);

SummaryRoutine summaryRoutine(&summaryService);
ReportRoutine reportRoutine(&reportService);
SimpleReportRoutine simpleReportRoutine(&simpleReportService);

std::map<uint8_t, IRoutine *> drifterServiceRoutines = {
    {Packet::OpCode::PING, &pingRoutine},
    {Packet::OpCode::CHANGE_OPERATION_MODE, &changeOpModeRoutine},
    {Packet::OpCode::SUMMARY_REPORT, &summaryRoutine},
    {Packet::OpCode::GET_REPORTING_PERIODS, &getRepolrtingPeriodsRoutine},
    {Packet::OpCode::SET_REPORTING_PERIODS, &setReportingPeriodsRoutine}    
};

std::map<uint8_t, IRoutine *> localizerServiceRoutines = {
    {Packet::OpCode::PING, &pingRoutine},
    {Packet::OpCode::CHANGE_OPERATION_MODE, &changeOpModeRoutine},
    {Packet::OpCode::SUMMARY_REPORT, &reportRoutine},
    {Packet::OpCode::SUMMARY_SIMPLE_REPORT, &simpleReportRoutine},
    {Packet::OpCode::GET_REPORTING_PERIODS, &getRepolrtingPeriodsRoutine},
    {Packet::OpCode::SET_REPORTING_PERIODS, &setReportingPeriodsRoutine}    
};


// Instancia del procesador de paquetes
PacketProcessor drifterPacketProcessor(&serialUSBDisplay, drifterServiceRoutines);
PacketProcessor localizerPacketProcessor(&serialUSBDisplay, localizerServiceRoutines);

// Tablas de enrutamiento
std::map<uint8_t, IPort *> localizerRoutes = {
    {(RECEIVER(Packet::Address::BACKEND) | Packet::PacketType::LORA_PACKET), &serialPort},
    {(RECEIVER(Packet::Address::BACKEND) | Packet::PacketType::IRIDIUM_PACKET), &serialPort},
    {(RECEIVER(Packet::Address::DRIFTER) | Packet::PacketType::LORA_PACKET), &loraPort},
    {(RECEIVER(Packet::Address::PI3) | Packet::PacketType::LORA_PACKET), &loraPort},
    {(RECEIVER(Packet::Address::DRIFTER) | Packet::PacketType::IRIDIUM_PACKET), &iridiumPort},
    {(RECEIVER(Packet::Address::PI3) | Packet::PacketType::IRIDIUM_PACKET), &iridiumPort}};
RoutingTable localizerRoutingTable(localizerRoutes);

std::map<uint8_t, IPort *> drifterRoutes = {
    {(RECEIVER(Packet::Address::BACKEND) | Packet::PacketType::LORA_PACKET), &loraPort},
    {(RECEIVER(Packet::Address::BACKEND) | Packet::PacketType::IRIDIUM_PACKET), &iridiumPort},
    {(RECEIVER(Packet::Address::LOCALIZER) | Packet::PacketType::LORA_PACKET), &loraPort},
    {(RECEIVER(Packet::Address::LOCALIZER) | Packet::PacketType::IRIDIUM_PACKET), &iridiumPort},
    {(RECEIVER(Packet::Address::PI3) | Packet::PacketType::LORA_PACKET), &serialPort},
    {(RECEIVER(Packet::Address::PI3) | Packet::PacketType::IRIDIUM_PACKET), &serialPort}};
RoutingTable drifterRoutingTable(drifterRoutes);



// Instancia de los enrutadores
auto localizerRouter = Router(Packet::Address::LOCALIZER,
                              &localizerRoutingTable,
                              &localizerPacketProcessor,
                              &serialUSBDisplay,
                              {&serialPort, &loraPort, &iridiumPort});

auto drifterRouter = Router(Packet::Address::DRIFTER,
                            &drifterRoutingTable,
                            &drifterPacketProcessor,
                            &serialUSBDisplay,
                            {&serialPort, &loraPort, &iridiumPort});

// Instancias de modos de operación (DRIFTER)
DrifterLaunchingMode drifterLaunchingMode(&serialUSBDisplay, &drifterRouter, &uBloxGPS,&adafruitLCBatteryController, &rtcController);
DrifterWorkingMode drifterWorkingMode(&serialUSBDisplay, &drifterRouter, &uBloxGPS, &adafruitLCBatteryController, &rtcController, &summaryService);
DrifterRecoveryMode drifterRecoveryMode(&serialUSBDisplay, &drifterRouter, &uBloxGPS,&adafruitLCBatteryController, &rtcController);

// Instancias de modos de operación (LOCALIZER)
LocalizerLaunchingMode localizerLaunchingMode(&adafruitDisplay, &localizerRouter, &mkrGPS, &simpleReportService, &rtcController);
LocalizerWorkingMode localizerWorkingMode(&adafruitDisplay, &localizerRouter, &mkrGPS, &reportService, &rtcController);
LocalizerRecoveryMode localizerRecoveryMode(&adafruitDisplay, &localizerRouter, &mkrGPS, &simpleReportService, &rtcController);
 

#endif // DEPENDENCIES_H