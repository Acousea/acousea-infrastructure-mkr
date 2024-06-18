#include "libraries.h"
#include <config.h>
#include "../lib/Communicator/ICommunicator.h"
#include "../lib/Communicator/LoraCommunicator.h"
#include "../lib/Communicator/IridiumCommunicator.h"
#include "../lib/SerialBridge/SerialBridge.h"
#include "../lib/Processor/MessageProcessor.h"

// Instancias de comunicadores
LoraCommunicator loraCommunicator;
IridiumCommunicator iridiumCommunicator;

// Instancia del procesador de mensajes
MessageProcessor messageProcessor(&display);

// Puntero al comunicador actual (Lora por defecto)
ICommunicator *currentCommunicator = &loraCommunicator;

// Instancia del puente serial
SerialBridge bridge(currentCommunicator, &messageProcessor);

void setup()
{
    // Inicializa la comunicación serial a 9600 baudios
    Serial.begin(9600);

    // Espera a que se establezca la conexión serial
    while (!Serial); // Espera sin hacer nada

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    { // Dirección I2C para el OLED
        Serial.println(F("SSD1306 allocation failed"));
        while (true);
    }
    // Print start message
    display.display();
    delay(2000); // Pausa inicial para la pantall
    display.clearDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.dim(true);
    display.setCursor(0, 0);
    display.print("Message Processor: ");
    display.display();
}

void loop()
{
    // Reenviar mensajes desde el puerto serial al comunicador
    bridge.relayFromSerial();

    // Pequeño retraso para evitar saturar el puerto serial
    delay(100);

    // Reenviar mensajes desde el comunicador al puerto serial
    bridge.relayToSerial();

    // Pequeño retraso para evitar saturar el puerto serial
    delay(100);


}
