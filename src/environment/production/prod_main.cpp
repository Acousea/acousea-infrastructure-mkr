#include "prod_main.h"

void prod_saveLocalizerConfig()
{
    acousea_NodeConfiguration localizerConfig = acousea_NodeConfiguration_init_default;

    localizerConfig.localAddress = 202;
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
    localizerConfig.operationModesModule.modes[0].reportTypeId = 0;
    localizerConfig.operationModesModule.activeModeId = 0;

    localizerConfig.operationModesModule.modes[0].has_transition = true;
    localizerConfig.operationModesModule.modes[0].transition = acousea_OperationModeTransition_init_default;
    localizerConfig.operationModesModule.modes[0].transition.duration = 2; // 0 means infinite
    localizerConfig.operationModesModule.modes[0].transition.targetModeId = 0; // Loop to itself


    localizerConfig.has_reportTypesModule = true;
    localizerConfig.reportTypesModule = acousea_ReportTypesModule_init_default;
    localizerConfig.reportTypesModule.reportTypes_count = 1;
    localizerConfig.reportTypesModule.reportTypes[0] = acousea_ReportType_init_default;
    localizerConfig.reportTypesModule.reportTypes[0].id = 0;
    snprintf(localizerConfig.reportTypesModule.reportTypes[0].name,
             sizeof(localizerConfig.reportTypesModule.reportTypes[0].name),
             "Default report");

    localizerConfig.reportTypesModule.reportTypes[0].includedModules_count = 1;
    localizerConfig.reportTypesModule.reportTypes[0].includedModules[0] = acousea_ModuleCode_LOCATION_MODULE;


    // Save the configuration
    nodeConfigurationRepository.saveConfiguration(localizerConfig);
}

void prod_saveDrifterConfig()
{
    acousea_NodeConfiguration drifterConfig = acousea_NodeConfiguration_init_default;
    drifterConfig.localAddress = 101;

    drifterConfig.has_operationModesModule = true;
    drifterConfig.operationModesModule = acousea_OperationModesModule_init_default;
    drifterConfig.operationModesModule.modes_count = 1;
    drifterConfig.operationModesModule.modes[0] = acousea_OperationMode_init_default;
    drifterConfig.operationModesModule.modes[0].id = 0;
    snprintf(drifterConfig.operationModesModule.modes[0].name,
             sizeof(drifterConfig.operationModesModule.modes[0].name),
             "Default mode");
    drifterConfig.operationModesModule.modes[0].reportTypeId = 0;
    drifterConfig.operationModesModule.activeModeId = 0;

    // Transition loop
    drifterConfig.operationModesModule.modes[0].has_transition = true;
    drifterConfig.operationModesModule.modes[0].transition = acousea_OperationModeTransition_init_default;
    drifterConfig.operationModesModule.modes[0].transition.duration = 2; // 0 means infinite
    drifterConfig.operationModesModule.modes[0].transition.targetModeId = 0; // Loop to itself

    // Report every 15 seconds
    drifterConfig.has_reportTypesModule = true;
    drifterConfig.reportTypesModule = acousea_ReportTypesModule_init_default;
    drifterConfig.reportTypesModule.reportTypes_count = 1;
    drifterConfig.reportTypesModule.reportTypes[0] = acousea_ReportType_init_default;
    drifterConfig.reportTypesModule.reportTypes[0].id = 0;
    snprintf(drifterConfig.reportTypesModule.reportTypes[0].name,
             sizeof(drifterConfig.reportTypesModule.reportTypes[0].name),
             "Default report");

    drifterConfig.reportTypesModule.reportTypes[0].includedModules_count = 3;
    drifterConfig.reportTypesModule.reportTypes[0].includedModules[0] = acousea_ModuleCode_BATTERY_MODULE;
    drifterConfig.reportTypesModule.reportTypes[0].includedModules[1] = acousea_ModuleCode_AMBIENT_MODULE;
    drifterConfig.reportTypesModule.reportTypes[0].includedModules[2] = acousea_ModuleCode_LOCATION_MODULE;


    // ---------------- LoRa e Iridium con 15s en modo 0 ----------------
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
    iridiumModule.entries[0].period = 1;

    drifterConfig.has_iridiumModule = true;
    drifterConfig.iridiumModule = iridiumModule;

    // Save the configuration
    nodeConfigurationRepository.saveConfiguration(drifterConfig);
}


void prod_setup()
{
#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: starting...\n");
#endif

#ifdef PLATFORM_ARDUINO
    // Inicializa la comunicación serial a 9600 baudios
    ConsoleSerial.begin(9600);
    // softwareSerialSercom0.begin(9600);
    // softwareSerialSercom1.begin(9600);
    delay(2000); // Espera a que el monitor serie esté listo

    pinPeripheral(PIN_A5, PIO_SERCOM_ALT); // TX
    pinPeripheral(PIN_A6, PIO_SERCOM_ALT); // RX
    pinPeripheral(PIN_SPI_MOSI, PIO_SERCOM); // TX
    pinPeripheral(PIN_SPI_SCK, PIO_SERCOM); // RX


    // Inicializa la pantalla Adafruit
    //    adafruitDisplay.init();

#endif

    // Inicializa el administrador de la tarjeta SD
    storageManagerRef.begin();

#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: Began storageManager\n");
#endif

    // Logger initialization and configuration
    Logger::initialize(
        &displayRef,
        &storageManagerRef,
        &rtcControllerRef,
        "log", // MAX 8 chars for 8.3 filenames
        Logger::Mode::Both
    );
    Logger::logInfo("================ Setting up Node =================");


    // Inicializa el administrador de energía
    // batteryController->init();

    // Inicializa el comunicador Serial
    // serialPort->init();

    // Inicializa el comunicador LoRa
    // loraPort->init();

    // Inicializa el comunicador Iridium
    // iridiumPort->init();

    // Inicializa el GPS
    // gps->init();
    // Logger::setCurrentTime(gps->getTimestamp());

    // Inicializa el controlador de tiempo real
    rtcControllerRef.init();
    rtcControllerRef.syncTime(gpsRef.getTimestamp());


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

    WatchdogUtils::enable(10000); // 10 seconds

    // Initialize the gps
    // gps->init();

#ifdef PLATFORM_ARDUINO
    solarXBatteryController.init();

    static MethodTask<BatteryProtectionPolicy> batteryProtectionTask(
        10000, // This must be less than watchdog timeout (10s)
        &batteryProtectionPolicy,
        &BatteryProtectionPolicy::enforce
    );
    scheduler.addTask(&batteryProtectionTask);

#endif
    nodeOperationRunner.init();

    static MethodTask<NodeOperationRunner> nodeOperationTask(
        15000, // 15 seconds
        &nodeOperationRunner,
        &NodeOperationRunner::run
    );
    scheduler.addTask(&nodeOperationTask);


#if MODE == DRIFTER_MODE
    // saveDrifterConfig();
#elif MODE == LOCALIZER_MODE
    // saveLocalizerConfig();
#endif
}

void prod_loop()
{
    scheduler.run();
}
#ifdef PLATFORM_ARDUINO

#if defined(PLATFORM_HAS_LORA)
void prod_onReceiveWrapper(int packetSize)
{
    SerialUSB.println("OnReceiveWrapper Callback");
    realLoraPort.onReceive(packetSize);
}
#endif

void prod_SERCOM0_Handler()
{
    // SerialUSB.println("INTERRUPT SERCOM0");
    softwareSerialSercom0.IrqHandler();
}

void prod_SERCOM1_Handler()
{
    // SerialUSB.println("INTERRUPT SERCOM0");
    softwareSerialSercom1.IrqHandler();
}


// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void prod_SERCOM3_Handler()
{
    mySerial3.IrqHandler();
}


#endif
