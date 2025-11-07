#include "test_dependencies.h"

// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef PLATFORM_ARDUINO
// Define los pines para el segundo puerto serie por hardware (SERCOM2)
// Uart softSerial(&sercom0, PIN_A4, PIN_A3, SERCOM_RX_PAD_1, UART_TX_PAD_0);
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
MockBatteryController mockBatteryController;
SolarXBatteryController solarXBatteryController(
    INA219_ADDRESS + 1, //  0x41 // Address of the battery sensor
    INA219_ADDRESS // 0x40 // Address of the solar panel sensor
    // INA219_ADDRESS + 4, // 0x44
    // INA219_ADDRESS + 5 // 0x45
);

IBatteryController& batteryControllerRef = solarXBatteryController; // o PMIC según HW

// --------- Display ----------
SerialArduinoDisplay serialUartDisplay(&ConsoleSerial);
IDisplay& displayRef = serialUartDisplay;


// --------- GPS ----------
// MKRGPS mkrGPS;
UBloxGNSS ubloxGNSS;
MockGPS mockGPS(0.0, 0.0, 1.0);
IGPS& gpsRef = ubloxGNSS;

// --------- RTC ----------
// MockRTCController mockRTCController;
// RTCController& rtcControllerRef = mockRTCController;
ZeroRTCController zeroRTCController;
RTCController& rtcControllerRef = zeroRTCController;

// --------- Storage ----------
SDStorageManager sdStorageManager;
StorageManager& storageManagerRef = sdStorageManager; // o hddStorageManager según build

// ---------- Puertos ----------
#ifdef PLATFORM_HAS_GSM
// Configuración GSM
GsmConfig gsmCfg(
    SECRET_PINNUMBER, // Tu SIM no tiene PIN
    SECRET_GPRS_APN, // APN del operador (ejemplo: Hologram)
    SECRET_GPRS_LOGIN, // Usuario del APN
    SECRET_GPRS_PASSWORD, // Password del APN
    AWS_MQTT_CLIENT_ID,
    AWS_MQTT_BROKER, // Host del servidor destino
    8883, // Puerto destino (8883 para MQTT sobre SSL)
    CLIENT_CERTIFICATE,
    CLIENT_PRIVATE_KEY
);
GsmMQTTPort gsmPort(gsmCfg);

#endif
PiController piPowerController;

BatteryProtectionPolicy batteryProtectionPolicy(batteryControllerRef, piPowerController);

// =======================================================
//       NATIVE BUILD
// =======================================================
#elif defined(PLATFORM_NATIVE)

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
IGPS* gps = &mockGPS;

// --------- RTC ----------
MockRTCController mockRTCController;
RTCController& rtcControllerRef = mockRTCController;

// --------- Storage ----------
HDDStorageManager hddStorageManager;
StorageManager& storageManagerRef = hddStorageManager; // o hddStorageManager según build

// --------- Router ----------
Router router({serialPort, loraPort, iridiumPort});

#else // NATIVE
#error "No valid PLATFORM defined. Please define PLATFORM as PLATFORM_ARDUINO or PLATFORM_NATIVE."

#endif // PLATFORM_NATIVE


TaskScheduler scheduler;


