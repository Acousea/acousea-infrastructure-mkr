#ifndef libraries_h
#define libraries_h

// ------------------ ARDUINO LIBRARIES ------------------
//#include <Arduino.h>
//#include "wiring_private.h"
//#include <SPI.h>
//#include <Wire.h>
//#include <SD.h>
//#include <LoRa.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//#include <Adafruit_LC709203F.h>
//#include <Arduino_MKRGPS.h>
//#include <Arduino_PMIC.h>
//#include <RTCZero.h>
//#include <IridiumSBD.h>
//#include <ArduinoJson.h>
//#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS


// ------------------ CUSTOM LIBRARIES ------------------
// ================== Shared ==================
#include <Result/Result.h>
#include <CRC/CRCUtils.h>
#include <SDManager/SDManager.h>
#include "Logger/Logger.h"
#include <ErrorHandler/ErrorHandler.h>
#include <ClassName.h>
// ============================================

#include <PMICBatteryController/PMICBatteryController.h>
#include <AdafruitLCBatteryController/AdafruitLCBatteryController.h>
//#include <SolarXBatteryController/SolarXBatteryController.h>

#include <MockGPS/MockGPS.h>
#include <MKRGPS/MKRGPS.h>
#include <UBloxGPS/UBloxGPS.h>

#include <RTCController.h>

#include <AdafruitDisplay/AdafruitDisplay.h>
#include <SerialUSBDisplay/SerialUSBDisplay.h>

#include <Ports/Serial/SerialPort.h>
#include <Ports/LoRa/LoRaPort.h>
#include <Ports/iridium/IridiumPort.h>
#include <Ports/LoRa/MockLoRaPort.h>
#include <Ports/iridium/MockIridiumPort.h>

#include <Routing/operationCode/OperationCode.h>
#include <Routing/Address/Address.h>
#include <Routing/RoutingChunk/RoutingChunk.h>

#include <Packet.h>
#include <Packets/BasicReportPacket.h>

#include <Payload/Payload.h>
#include <Payload/Payloads/setConfig/NewNodeConfigurationPayload.h>
#include <Payload/Payloads/getConfig/GetUpdatedNodeConfigurationPayload.h>
#include <Payload/Payloads/complete/CompleteStatusReportPayload.h>
#include <Payload/Payloads/basic/BasicStatusReportPayload.h>
#include <Payload/Payloads/error/ErrorPayload.h>

#include <Payload/Modules/SerializableModule.h>
#include <Payload/Modules/JSONSerializable.h>
#include <Payload/Modules/implementation/ambient/AmbientModule.h>
#include <Payload/Modules/implementation/battery/BatteryModule.h>
#include <Payload/Modules/implementation/location/LocationModule.h>
#include <Payload/Modules/implementation/network/NetworkModule.h>
#include <Payload/Modules/implementation/operationModes/OperationModesModule.h>
#include <Payload/Modules/implementation/operationModes/OperationModesGraphModule.h>
#include <Payload/Modules/implementation/reporting/reportingConfiguration/ReportingConfiguration.h>
#include <Payload/Modules/implementation/reporting/ReportingModule.h>
#include <Payload/Modules/implementation/reporting/loRa/LoRaReportingPeriodModule.h>
#include <Payload/Modules/implementation/reporting/iridium/IridiumReportingPeriodModule.h>
#include <Payload/Modules/implementation/operationModes/OperationMode.h>
#include <Payload/Modules/implementation/operationModes/OperationModesModule.h>
#include <Payload/Modules/implementation/operationModes/OperationModesGraphModule.h>

#include <Payload/Modules/implementation/rtc/RTCModule.h>
#include <Payload/Modules/implementation/storage/StorageModule.h>

#include <Payload/Modules/pamModules/PamModule.h>
#include <Payload/Modules/pamModules/ICListen/ICListenHF.h>
#include <Payload/Modules/pamModules/ICListen/implementation/status/ICListenStatus.h>
#include <Payload/Modules/pamModules/ICListen/implementation/logging/ICListenLogigngConfig.h>
#include <Payload/Modules/pamModules/ICListen/implementation/streaming/ICListenStreamingConfig.h>
#include <Payload/Modules/pamModules/ICListen/implementation/stats/ICListenRecordingStats.h>

#include <Payload/Modules/factory/ModuleFactory.h>

#include <Router.h>

#include <routines/SetNodeConfigurationRoutine/SetNodeConfigurationRoutine.h>
#include <routines/CompleteSummaryReportRoutine/CompleteSummaryReportRoutine.h>
#include <routines/BasicSummaryReportRoutine/BasicSummaryReportRoutine.h>

#include <NodeConfiguration/NodeConfiguration.h>
#include <NodeConfigurationRepository/NodeConfigurationRepository.h>

#include <NodeOperationRunner/NodeOperationRunner.h>


#endif