#include "dependencies.h"

//#define DRIFTER_MODE 0
//#define LOCALIZER_MODE 1
//#define MODE LOCALIZER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario
//#define MODE DRIFTER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario


void setup() {
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

    // Inicializa la pantalla Adafruit
//    adafruitDisplay.init();

    // Initialize Logger in SerialOnly mode
    Logger::initialize(&sdManager, "/log.csv", Logger::Mode::SerialOnly);

    // Inicializa el administrador de la tarjeta SD
    sdManager.begin();

    // Set a custom error handler
//    ErrorHandler::setHandler([](const std::string &message) {
//        Serial.println("Custom handler invoked!");
//        Serial.println(message.c_str());
//    });

    // Log an error
//    ErrorHandler::handleError("Failed to initialize module.");


    // Resets the reporting periods to the default values
    nodeConfigurationRepository.reset();

    // Inicializa el repositorio de configuración
    nodeConfigurationRepository.init();

    // Inicializa el comunicador Serial
    serialPort.init();

    // Inicializa el GPS
//    gps->init();

    // Inicializa el controlador de tiempo real
//    rtcController.init();
//    rtcController.syncTime(gps->getTimestamp());

    // Inicializa el administrador de energía
    adafruitLCBatteryController.init();

    // Inicializa el comunicador LoRa
    realLoraPort.init();

    // Inicializa el comunicador Iridium
    realIridiumPort.init();

}

void loop() {


    static unsigned long lastTime = 0;
    // Operate every 1 second
    if (millis() - lastTime >= 1000) {
        lastTime = millis();
        SerialUSB.println("Operating Node...");
        nodeOperationRunner.init();
//        nodeOperationRunner.run();
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

