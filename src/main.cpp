#include "dependencies.h"

#define DRIFTER_MODE 0
#define LOCALIZER_MODE 1
// #define MODE LOCALIZER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario
#define MODE DRIFTER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario


void setup() {
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

//    NodeOperationRunner *opRunner = new NodeOperationRunner(display,
//                                                            router,
//                                                            packetProcessor,
//                                                            reportingRoutines,
//                                                            nodeConfigurationRepository
//    );
//
//    NodeOperationRunner nodeOperationRunner(display,
//                                        router,
//                                        packetProcessor,
//                                        reportingRoutines,
//                                        nodeConfigurationRepository
//);



    // Inicializa la pantalla Adafruit
    // adafruitDisplay.init();

    // Inicializa el administrador de la tarjeta SD
//    sdManager.begin();

    // Resets the reporting periods to the default values
//    nodeConfigurationRepository.reset();

    // Inicializa el repositorio de configuración
//    nodeConfigurationRepository.begin();

    // Inicializa el comunicador Serial
//    serialPort.init()

    // Inicializa el GPS
    gps->init();

    // Inicializa el controlador de tiempo real
    rtcController.init();
    rtcController.syncTime(gps->getTimestamp());

    // Inicializa el administrador de energía
    adafruitLCBatteryController.init();

    // Inicializa el comunicador LoRa
//    loraPort.init();

    // Inicializa el comunicador Iridium
//    iridiumPort.init();

    // Inicializa el runner
//    nodeOperationRunner.init();
}

void loop() {


    static unsigned long lastTime = 0;
    // Operate every 1 second
    if (millis() - lastTime >= 1000) {
        lastTime = millis();
        SerialUSB.println("Operating Node...");
//        nodeOperationRunner.run();
    }
}

void onReceiveWrapper(int packetSize) {
    SerialUSB.println("OnReceiveWrapper Callback");
    loraPort.onReceive(packetSize);
}

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
//void SERCOM3_Handler() {
//    mySerial3.IrqHandler();
//}

