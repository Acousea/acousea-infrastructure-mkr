#include "dependencies.h"
#include "time/getMillis.hpp"
// #include "../lib/MockLib/include/library.h"

#define ENSURE(ptr, name) do{ if(!(ptr)){ std::fprintf(stderr,"[native] %s is NULL\n", name); return; } }while(0)


#define DRIFTER_MODE 0
#define LOCALIZER_MODE 1
// #define MODE LOCALIZER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario
#define MODE DRIFTER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario

void saveLocalizerConfig()
{
    acousea_NodeConfiguration localizerConfig = acousea_NodeConfiguration_init_default;

    localizerConfig.localAddress = 2;
    localizerConfig.has_loraModule = false;
    localizerConfig.has_iridiumModule = false;

    localizerConfig.has_operationModesModule = true;
    localizerConfig.operationModesModule = acousea_OperationModesModule_init_default;
    localizerConfig.operationModesModule.modes_count = 1;
    localizerConfig.operationModesModule.modes[0] = acousea_OperationMode_init_default;
    localizerConfig.operationModesModule.modes[0].id = 0;
    snprintf(localizerConfig.operationModesModule.modes[0].name,
             sizeof(localizerConfig.operationModesModule.modes[0].name),
             "Default mode");

    localizerConfig.has_operationGraphModule = true;
    localizerConfig.operationGraphModule = acousea_OperationModesGraphModule_init_default;
    localizerConfig.operationGraphModule.graph_count = 1;
    localizerConfig.operationGraphModule.graph[0] = acousea_OperationModesGraphModule_GraphEntry_init_default;
    localizerConfig.operationGraphModule.graph[0].key = 0;
    localizerConfig.operationGraphModule.graph[0].has_value = true;
    localizerConfig.operationGraphModule.graph[0].value.targetMode = 0;
    localizerConfig.operationGraphModule.graph[0].value.duration = 3;
    // Save the configuration

    nodeConfigurationRepository.saveConfiguration(localizerConfig);
}

void saveDrifterConfig()
{
    acousea_NodeConfiguration drifterConfig = acousea_NodeConfiguration_init_default;
    drifterConfig.localAddress = 1;

    drifterConfig.has_operationModesModule = true;
    drifterConfig.operationModesModule = acousea_OperationModesModule_init_default;
    drifterConfig.operationModesModule.modes_count = 1;
    drifterConfig.operationModesModule.modes[0] = acousea_OperationMode_init_default;
    drifterConfig.operationModesModule.modes[0].id = 0;
    snprintf(drifterConfig.operationModesModule.modes[0].name,
             sizeof(drifterConfig.operationModesModule.modes[0].name),
             "Default mode");


    drifterConfig.has_operationGraphModule = true;
    drifterConfig.operationGraphModule = acousea_OperationModesGraphModule_init_default;
    drifterConfig.operationGraphModule.graph_count = 1;
    drifterConfig.operationGraphModule.graph[0] = acousea_OperationModesGraphModule_GraphEntry_init_default;
    drifterConfig.operationGraphModule.graph[0].key = 0;
    drifterConfig.operationGraphModule.graph[0].has_value = true;
    drifterConfig.operationGraphModule.graph[0].value.targetMode = 0;
    drifterConfig.operationGraphModule.graph[0].value.duration = 1;

    acousea_LoRaReportingModule loraModule = acousea_LoRaReportingModule_init_default;
    loraModule.entries_count = 1;
    loraModule.entries[0] = acousea_ReportingPeriodEntry_init_default;
    loraModule.entries[0].modeId = 0;
    loraModule.entries[0].period = 15;

    drifterConfig.has_loraModule = true;
    drifterConfig.loraModule = loraModule;

    acousea_IridiumReportingModule iridiumModule = acousea_IridiumReportingModule_init_default;
    iridiumModule.entries_count = 1;
    iridiumModule.entries[0] = acousea_ReportingPeriodEntry_init_default;
    iridiumModule.entries[0].modeId = 0;
    iridiumModule.entries[0].period = 15;

    drifterConfig.has_iridiumModule = true;
    drifterConfig.iridiumModule = iridiumModule;

    // Save the configuration
    nodeConfigurationRepository.saveConfiguration(drifterConfig);
}


void setup()
{
#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: starting...\n");
#endif
    ENSURE(storageManager, "storageManager");
    ENSURE(display, "display");
    ENSURE(gps, "gps");
    ENSURE(rtcController, "rtcController");
    ENSURE(battery, "battery");
    ENSURE(serialPort, "serialPort");
    ENSURE(loraPort, "loraPort");
    ENSURE(iridiumPort, "iridiumPort");

#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: Ensured pointers\n");
#endif

#ifdef ARDUINO
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);
    delay(2000); // Espera a que el monitor serie esté listo
#endif

    // Test the mock library
    // MockLibrary::print_with_callback([](const char *message) {
    //     Serial.println(message);
    // });

    // Inicializa la pantalla Adafruit
    //    adafruitDisplay.init();

    // Inicializa el administrador de la tarjeta SD
    if (!storageManager->begin())
    {
        ErrorHandler::handleError("Failed to initialize SD card.");
    }

#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: Began storageManager\n");
#endif

    // Logger initialization and configuration
    Logger::initialize(
        display,
        storageManager,
        "log",
        Logger::Mode::Both
    );
    Logger::logInfo("================ Setting up Node =================");

    // Initialize the iclistenServicePtr
#if MODE == DRIFTER_MODE
    icListenServicePtr->init();
#endif

    // Inicializa el GPS
    gps->init();
    Logger::setCurrentTime(gps->getTimestamp());

    // Inicializa el controlador de tiempo real
    rtcController->init();
    rtcController->syncTime(gps->getTimestamp());


    // Set a custom error handler
    //    ErrorHandler::setHandler([](const std::string &message) {
    //        Serial.println("Custom handler invoked!");
    //        Serial.println(message.c_str());
    //    });

    // Log an error
    //    ErrorHandler::handleError("Failed to initialize module.");


    // Resets the reporting periods to the default values
    //    nodeConfigurationRepository.reset();

    // Inicializa el repositorio de configuración
    nodeConfigurationRepository.init();
#if MODE == DRIFTER_MODE
    saveDrifterConfig();
#elif MODE == LOCALIZER_MODE
    saveLocalizerConfig();
#endif

    // Inicializa el comunicador Serial
    serialPort->init();

    // Inicializa el administrador de energía
    battery->init();

    // Inicializa el comunicador LoRa
    loraPort->init();

    // Inicializa el comunicador Iridium
    iridiumPort->init();
}

void loop()
{
    static unsigned long lastTime = 0;
    // Operate every 30 seconds
    if (getMillis() - lastTime >= 15000 || lastTime == 0)
    {
        nodeOperationRunner.init();
        nodeOperationRunner.run();
        lastTime = getMillis();
    }
}

#ifdef ARDUINO

void onReceiveWrapper(int packetSize){
    SerialUSB.println("OnReceiveWrapper Callback");
    realLoraPort.onReceive(packetSize);
}

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void SERCOM3_Handler(){
    mySerial3.IrqHandler();
}

#endif


// #if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
// #include "dependencies.h"
// #include <windows.h>
//
// #include <chrono>
// #include <thread>
// #include <cstdio>
//
// static inline void delay_ms(unsigned ms) {
//     std::this_thread::sleep_for(std::chrono::milliseconds(ms));
// }
//
// // int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
// int main() {
//     setvbuf(stdout, nullptr, _IONBF, 0);
//     setvbuf(stderr, nullptr, _IONBF, 0);
//     std::printf("[native] WinMain: starting...\n");
//
//     setup();
//     std::printf("[native] setup() done. Entering loop...\n");
//
//     for (;;) {
//         loop();
//         delay_ms(1); // evita 100% CPU
//     }
// }
// #endif
//
