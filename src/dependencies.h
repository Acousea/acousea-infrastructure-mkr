
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
#include "../lib/RoutingTable/RoutingTable.h"

#include "../lib/Battery/PMICBatteryController.h"
#include "../lib/Battery/AdafruitLCBatteryController.h"

#include "../GPS/MockGPS/MockGPS.h"
#include "../GPS/MKRGPS/MKRGPS.h"
#include "../GPS/UBloxGPS/UBloxGPS.h"
#include "../lib/RTC/RTCController.h"

#include "../lib/OperationManager/OperationManager.h"

#include "../lib/Services/SummaryService.h"

#include "../lib/OperationModes/Drifter/LaunchingMode.h"
#include "../lib/OperationModes/Drifter/WorkingMode.h"
#include "../lib/OperationModes/Drifter/RecoveryMode.h"

#include "../lib/OperationModes/Localizer/LaunchingMode.h"
#include "../lib/OperationModes/Localizer/WorkingMode.h"
#include "../lib/OperationModes/Localizer/RecoveryMode.h"

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

// Instancia del administrador de operaciones
OperationManager operationManager;

// Instancias de rutinas de servicio
PingRoutine pingRoutine;
ChangeOperationModeRoutine changeOpModeRoutine(operationManager);
SummaryService summaryService;
SummaryRoutine summaryRoutine(&summaryService);
std::map<uint8_t, IRoutine *> serviceRoutines = {
    {Packet::OpCode::PING, &pingRoutine}, 
    {Packet::OpCode::CHANGE_OPERATION_MODE, &changeOpModeRoutine},
    {Packet::OpCode::SUMMARY_REPORT, &summaryRoutine}
};

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

// Instancia del procesador de paquetes
PacketProcessor packetProcessor(&serialUSBDisplay, serviceRoutines);

// Instancia de los enrutadores
auto localizerRouter = Router(Packet::Address::LOCALIZER,
                              &localizerRoutingTable,
                              &packetProcessor,
                              &serialUSBDisplay,
                              {&serialPort, &loraPort, &iridiumPort});

auto drifterRouter = Router(Packet::Address::DRIFTER,
                            &drifterRoutingTable,
                            &packetProcessor,
                            &serialUSBDisplay,
                            {&serialPort, &loraPort, &iridiumPort});



// Instancias de modos de operación (DRIFTER)
DrifterLaunchingMode drifterLaunchingMode(&serialUSBDisplay, &drifterRouter, &mockGPS,  &rtcController);
DrifterWorkingMode drifterWorkingMode(&serialUSBDisplay, &drifterRouter, &mockGPS, &rtcController,  &summaryService);
DrifterRecoveryMode drifterRecoveryMode(&serialUSBDisplay, &drifterRouter, &mockGPS,  &rtcController);

// Instancias de modos de operación (LOCALIZER)
LocalizerLaunchingMode localizerLaunchingMode(&serialUSBDisplay, &localizerRouter, &mockGPS);
LocalizerWorkingMode localizerWorkingMode(&serialUSBDisplay, &localizerRouter, &mockGPS);
LocalizerRecoveryMode localizerRecoveryMode(&serialUSBDisplay, &localizerRouter, &mockGPS);
#endif // DEPENDENCIES_H