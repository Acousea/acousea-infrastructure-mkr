#include "dependencies.h"

#define DRIFTER_MODE 0
#define LOCALIZER_MODE 1
// #define MODE LOCALIZER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario
#define MODE DRIFTER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario

void saveLocalizerConfig() {
    nodeConfigurationRepository.saveConfiguration(
        NodeConfiguration(
            Address(2),
            OperationModesGraphModule::from({
                {
                    0,
                    OperationModesGraphModule::Transition(0, 3)
                },
            }),
            std::nullopt,
            std::nullopt
        ));
}

void saveDrifterConfig() {
    nodeConfigurationRepository.saveConfiguration(
        NodeConfiguration(
            Address(2),
            OperationModesGraphModule::from({
                {
                    0,
                    OperationModesGraphModule::Transition(0, 3)
                },
            }),
            LoRaReportingModule(
                {
                    {
                        0, ReportingConfiguration(
                            0, 15,
                            ReportingConfiguration::ReportType::BASIC
                        )
                    }
                }),
            IridiumReportingModule(
                {
                    {
                        0,
                        ReportingConfiguration(
                            0, 15,
                            ReportingConfiguration::ReportType::COMPLETE)
                    }
                }
            )

        ));
}


void setup() {
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

    // Inicializa la pantalla Adafruit
    //    adafruitDisplay.init();

    // Inicializa el administrador de la tarjeta SD
    if (!sdManager.begin()) {
        ErrorHandler::handleError("Failed to initialize SD card.");
    }

    // Inicializa el GPS
    gps->init();

    // Inicializa el controlador de tiempo real
    rtcController.init();
    rtcController.syncTime(gps->getTimestamp());

    // Logger initialization and configuration
    Logger::initialize(&sdManager, "/log.csv", Logger::Mode::Both);
    Logger::logInfo("================ Setting up Node =================");
    Logger::setCurrentTime(gps->getTimestamp());

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
    serialPort.init();

    // Inicializa el administrador de energía
    adafruitLCBatteryController.init();

    // Inicializa el comunicador LoRa
    realLoraPort.init();

    // Inicializa el comunicador Iridium
#if MODE == DRIFTER_MODE
    // realIridiumPort.init();
    mockIridiumPort.init();
#elif MODE == LOCALIZER_MODE
    mockIridiumPort.init();
#endif
}

void loop() {
    static unsigned long lastTime = 0;
    // Operate every 30 seconds
    if (millis() - lastTime >= 30000 || lastTime == 0) {
        lastTime = millis();
        nodeOperationRunner.init();
        nodeOperationRunner.run();
        nodeOperationRunner.finish();
    }
}

void onReceiveWrapper(int packetSize) {
    SerialUSB.println("OnReceiveWrapper Callback");
    realLoraPort.onReceive(packetSize);
}

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void SERCOM3_Handler() {
    mySerial3.IrqHandler();
}
