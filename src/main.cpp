#include "dependencies.h"

void setupDrifter(){
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

    // Inicializa la pantalla Adafruit
    // adafruitDisplay.init();

    // Inicializa el comunicador Serial
    serialPort.init();

    // Inicializa el administrador de energía
    adafruitLCBatteryController.init();

    // Inicializa el comunicador LoRa
    loraPort.init();

    // Inicializa el comunicador Iridium
    iridiumPort.init();


}

void operateDrifter(){
    SerialUSB.println("Operating Drifter...");
    switch (operationManager.getMode()) {
        case LAUNCHING_MODE:
            drifterLaunchingMode.run();
            break;
        case WORKING_MODE:
            drifterWorkingMode.run();
            break;
        case RECOVERY_MODE:
            drifterRecoveryMode.run();
            break;
        default:
            SerialUSB.println("Unknown operation mode");            
            operationManager.setMode(LAUNCHING_MODE);
            break;
        }
}

void setupLocalizer(){

    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

    // Inicializa la pantalla Adafruit
    // adafruitDisplay.init();

    // Inicializa el comunicador Serial
    serialPort.init();

    // Inicializa el administrador de energía
    pmicBatteryController.init();

    // Inicializa el comunicador LoRa
    loraPort.init();

    // Inicializa el comunicador Iridium
    iridiumPort.init();


}

void operateLocalizer(){
    SerialUSB.println("Operating Localizer...");
    switch (operationManager.getMode()) {
        case LAUNCHING_MODE:
            localizerLaunchingMode.run();
            break;
        case WORKING_MODE:
            localizerWorkingMode.run();
            break;
        case RECOVERY_MODE:
            localizerRecoveryMode.run();
            break;
        default:
            SerialUSB.println("Unknown operation mode");            
            operationManager.setMode(LAUNCHING_MODE);
            break;
        }
}

void setup()
{   
    setupDrifter();
    // setupLocalizer();       
}


// Reenviar mensajes desde el puerto serial al comunicador y viceversa
// SerialUSB.println("Drifter Listening...");
// drifterRouter.relayPorts();
// SerialUSB.println("Localizer Listening...");
// localizerRouter.relayPorts();

void loop()
{
    static long lastTime = 0;    
    // Operate every 1 second
    if (millis() - lastTime >= 1000) {        
        lastTime = millis();        
        operateDrifter();
        // operateLocalizer();      
    }    
}

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void SERCOM3_Handler()
{
    mySerial3.IrqHandler();
}

void onReceiveWrapper(int packetSize)
{
    SerialUSB.println("OnReceiveWrapper Callback");
    loraPort.onReceive(packetSize);
}