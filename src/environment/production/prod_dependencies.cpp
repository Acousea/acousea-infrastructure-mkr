#include "prod_dependencies.h"

#include "environment/credentials.hpp"

// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef PLATFORM_ARDUINO
Uart softwareSerialSercom0(&sercom0,
                           PIN_A6, // RX
                           PIN_A5, // TX
                           SERCOM_RX_PAD_3,
                           UART_TX_PAD_2
);

Uart softwareSerialSercom1(&sercom1,
                           PIN_SPI_SCK, // RX
                           PIN_SPI_MOSI, // TX
                           SERCOM_RX_PAD_1,
                           UART_TX_PAD_0
);


// --------- Batería ----------
// PMICBatteryController pmicBatteryController;
// AdafruitLCBatteryController adafruitLCBatteryController;
// MockBatteryController mockBatteryController;
SolarXBatteryController solarXBatteryController(
    INA219_ADDRESS + 1, //  0x41 // Address of the battery sensor
    INA219_ADDRESS // 0x40 // Address of the solar panel sensor
    // INA219_ADDRESS + 4, // 0x44
    // INA219_ADDRESS + 5 // 0x45
);
IBatteryController& batteryControllerRef = solarXBatteryController; // o PMIC según HW

// --------- Display ----------
// AdafruitDisplay adafruitDisplay;
SerialArduinoDisplay serialUSBDisplay(&ConsoleSerial);
IDisplay& displayRef = serialUSBDisplay;

// --------- Puertos ----------
SerialPort realSerialPort(&Serial1, 4800);

#ifdef PLATFORM_HAS_LORA
MockLoRaPort mockLoraPort;
LoraPort realLoraPort;
#endif

MockIridiumPort mockIridiumPort;
IridiumPort realIridiumPort;

#ifdef PLATFORM_HAS_GSM
// Configuración GSM

GsmConfig gsmCfg(
    SECRET_PINNUMBER,
    SECRET_GPRS_APN,
    SECRET_GPRS_LOGIN,
    SECRET_GPRS_PASSWORD,
    AWS_MQTT_CLIENT_ID,
    AWS_MQTT_BROKER,
    8883,
    CLIENT_CERTIFICATE,
    CLIENT_PRIVATE_KEY
);


GsmMQTTPort gsmPort(gsmCfg);
#endif


IPort* serialPort = &realSerialPort;

#ifdef PLATFORM_HAS_LORA
IPort* loraOrGsmPort = &mockLoraPort;
#elif defined(PLATFORM_HAS_GSM)
IPort* loraOrGsmPort = &gsmPort;
#endif

IPort* iridiumPort = &mockIridiumPort;

// --------- GPS ----------
// MockGPS mockGPS(0.0, 0.0, 1.0);
// IGPS* gps = &mockGPS;
// MKRGPS mkrGPS;
// IGPS* gps = &mkrGPS;
UBloxGNSS uBloxGPS;
IGPS& gpsRef = uBloxGPS;

// --------- RTC ----------
ZeroRTCController zeroRTCController;
// MockRTCController mockRTCController;
RTCController& rtcControllerRef = zeroRTCController;

// --------- Storage ----------
SDStorageManager sdStorageManager;
StorageManager& storageManagerRef = sdStorageManager; // o hddStorageManager según build

// --------- Router ----------
Router router({serialPort, loraOrGsmPort, iridiumPort});

// --------- Power ----------
MosfetController mosfetController;
PiController rockPiPowerController;
BatteryProtectionPolicy batteryProtectionPolicy(batteryControllerRef, rockPiPowerController);

// =======================================================
//       NATIVE BUILD
// =======================================================
#else // NATIVE

// --------- Batería ----------
MockBatteryController mockBatteryController;
IBatteryController& batteryControllerRef = mockBatteryController;

// --------- Display ----------
ConsoleDisplay consoleDisplay;
IDisplay& displayRef = consoleDisplay;

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
IGPS& gpsRef = mockGPS;

// --------- RTC ----------
MockRTCController mockRTCController;
RTCController& rtcControllerRef = mockRTCController;

// --------- Storage ----------
HDDStorageManager hddStorageManager;
StorageManager& storageManagerRef = hddStorageManager; // o hddStorageManager según build

// --------- Router ----------
Router router({serialPort, loraPort, iridiumPort});


#endif // ARDUINO vs NATIVE

// =======================================================
//       COMÚN A AMBOS
// =======================================================


NodeConfigurationRepository nodeConfigurationRepository(storageManagerRef);

// ============================================================
// =========== Device port mappings ===========================
// ============================================================

// Mapa con ambos dispositivos (ICListen + VR2C)
const std::unordered_map<ModuleProxy::DeviceAlias, IPort::PortType> fullDevicePortMap = {
    {ModuleProxy::DeviceAlias::ICListen, IPort::PortType::SerialPort},
    {ModuleProxy::DeviceAlias::VR2C, IPort::PortType::SerialPort}
};

// Mapa con solo ICListen
const std::unordered_map<ModuleProxy::DeviceAlias, IPort::PortType> iclistenOnlyPortMap = {
    {ModuleProxy::DeviceAlias::ICListen, IPort::PortType::SerialPort}
};

const std::unordered_map<ModuleProxy::DeviceAlias, IPort::PortType> emptyDevicePortMap = {};

ModuleProxy moduleProxy(router, fullDevicePortMap);

SetNodeConfigurationRoutine setNodeConfigurationRoutine(nodeConfigurationRepository, moduleProxy);
// SetNodeConfigurationRoutine setNodeConfigurationRoutine(nodeConfigurationRepository, std::nullopt);

GetUpdatedNodeConfigurationRoutine getUpdatedNodeConfigurationRoutine(nodeConfigurationRepository,
                                                                      moduleProxy,
                                                                      gpsRef,
                                                                      batteryControllerRef,
                                                                      rtcControllerRef
);


CompleteStatusReportRoutine completeStatusReportRoutine(nodeConfigurationRepository,
                                                        moduleProxy,
                                                        gpsRef,
                                                        batteryControllerRef,
                                                        rtcControllerRef
);


StoreNodeConfigurationRoutine storeNodeConfigurationRoutine(
    nodeConfigurationRepository,
    moduleProxy
);

// std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> commandRoutines = {
//     {acousea_CommandBody_setConfiguration_tag, &setNodeConfigurationRoutine},
//     {acousea_CommandBody_requestedConfiguration_tag, &getUpdatedNodeConfigurationRoutine},
// };
//
// std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> responseRoutines = {
//     {acousea_ResponseBody_setConfiguration_tag, &storeNodeConfigurationRoutine},
//     {acousea_ResponseBody_updatedConfiguration_tag, &storeNodeConfigurationRoutine},
// };
//
//
// std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> reportRoutines = {
//     {acousea_ReportBody_statusPayload_tag, &completeStatusReportRoutine},
// };
//
//
// NodeOperationRunner nodeOperationRunner(
//     router,
//     nodeConfigurationRepository,
//     commandRoutines, responseRoutines, reportRoutines
// );
//

// Mapa jerárquico de routines
std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> routines = {
    {
        acousea_CommunicationPacket_command_tag, {
                {acousea_CommandBody_setConfiguration_tag, &setNodeConfigurationRoutine},
                {acousea_CommandBody_requestedConfiguration_tag, &getUpdatedNodeConfigurationRoutine},
            }
    },
    {
        acousea_CommunicationPacket_response_tag, {
                {acousea_ResponseBody_setConfiguration_tag, &storeNodeConfigurationRoutine},
                {acousea_ResponseBody_updatedConfiguration_tag, &storeNodeConfigurationRoutine},
            }
    },
    {
        acousea_CommunicationPacket_report_tag, {
                {acousea_ReportBody_statusPayload_tag, &completeStatusReportRoutine},
            }
    }
};

NodeOperationRunner nodeOperationRunner(
    router,
    nodeConfigurationRepository,
    routines
);

TaskScheduler scheduler;