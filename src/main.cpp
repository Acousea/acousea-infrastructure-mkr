#include "libraries.h"
#include <config.h>
#include "../lib/Communicator/ICommunicator.h"
#include "../lib/Communicator/LoraCommunicator.h"
#include "../lib/Communicator/IridiumCommunicator.h"
#include "../lib/Communicator/SerialCommunicator.h"
#include "../lib/SerialBridge/SerialBridge.h"
#include "../lib/Processor/MessageProcessor.h"
#include "../lib/Display/AdafruitDisplay.h"
#include "../lib/Display/SerialUSBDisplay.h"

// Must define OperationMode class to manage the state of the system

// Instancias de comunicadores
LoraCommunicator loraCommunicator;
IridiumCommunicator iridiumCommunicator;                 // FIXME:  Iridium Communicator must have mySerial3 injection
SerialCommunicator serialCommunicator(&mySerial3, 9600); // Serial1 is the hardware serial port PINS 0 and 1

// // Instancia de la pantalla
AdafruitDisplay adafruitDisplay(&adafruit_SSD1306);
SerialUSBDisplay serialUSBDisplay;

// // Instancia del procesador de mensajes
MessageProcessor messageProcessor(&serialUSBDisplay);

// // Puntero al comunicador actual (Lora por defecto)
ICommunicator *currentCommunicator = &iridiumCommunicator;

// Instancia del puente serial
CommunicatorRelay relay(&serialCommunicator, currentCommunicator, &messageProcessor);

void setup()
{
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

    // Inicializa el comunicador Serial
    serialCommunicator.init();

    // Inicializa la pantalla Adafruit
    // adafruitDisplay.init();
}

void loop()
{
    static long lastTime = 0;
    // Print debugging message every 1 second
    if (millis() - lastTime > 1000)
    {
        lastTime = millis();
        // serialUSBDisplay.print("Listening...");
        SerialUSB.println("Listening...");
    }

    // Reenviar mensajes desde el puerto serial al comunicador
    relay.relayFromPrimary();

    // Pequeño retraso para evitar saturar el puerto serial
    delay(100);

    // Reenviar mensajes desde el comunicador al puerto serial
    relay.relayFromSecondary();

    // Pequeño retraso para evitar saturar el puerto serial
    delay(100);
}

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void SERCOM3_Handler()
{
    mySerial3.IrqHandler();
}