#include "test_dependencies.h"
// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef ARDUINO
// --------- Batería ----------
MockBatteryController mockBatteryController;
SolarXBatteryController solarXBatteryController(std::vector<uint8_t>{0x40, 0x41});
IBatteryController* batteryController = &solarXBatteryController; // o PMIC según HW

// --------- Display ----------
SerialUSBDisplay serialUSBDisplay;
IDisplay* display = &serialUSBDisplay;


// --------- GPS ----------
MockGPS mockGPS(0.0, 0.0, 1.0);
IGPS* gps = &mockGPS;

// --------- RTC ----------
MockRTCController mockRTCController;
RTCController* rtcController = &mockRTCController;

// --------- Storage ----------
SDStorageManager sdStorageManager;
StorageManager* storageManager = &sdStorageManager; // o hddStorageManager según build


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
