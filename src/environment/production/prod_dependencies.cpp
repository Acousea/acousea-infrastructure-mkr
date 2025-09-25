#include "prod_dependencies.h"

#include "environment/credentials.hpp"


// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef ARDUINO
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
IBatteryController* batteryController = &solarXBatteryController; // o PMIC según HW

// --------- Display ----------
// AdafruitDisplay adafruitDisplay;
SerialArduinoDisplay serialUSBDisplay(&ConsoleSerial);
IDisplay* display = &serialUSBDisplay;

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
GsmConfig gsmCfg = {
    .pin = SECRET_PINNUMBER, // Tu SIM no tiene PIN
    .apn = SECRET_GPRS_APN, // APN del operador (ejemplo: Hologram)
    .user = SECRET_GPRS_LOGIN, // Usuario del APN
    .pass = SECRET_GPRS_PASSWORD, // Password del APN
    .clientId = AWS_MQTT_CLIENT_ID,
    .broker = AWS_MQTT_BROKER, // Host del servidor destino
    .port = 8883, // Puerto destino (8883 para MQTT sobre SSL)
    .certificate = CLIENT_CERTIFICATE
};

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
// MKRGPS mkrGPS;
UBloxGNSS uBloxGPS;
IGPS* gps = &uBloxGPS;

// --------- RTC ----------
ZeroRTCController zeroRTCController;
// MockRTCController mockRTCController;
RTCController* rtcController = &zeroRTCController;

// --------- Storage ----------
SDStorageManager sdStorageManager;
StorageManager* storageManager = &sdStorageManager; // o hddStorageManager según build

// --------- Router ----------
Router router({serialPort, loraOrGsmPort, iridiumPort});



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
