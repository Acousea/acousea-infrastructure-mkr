#include "libraries.h"
#include <config.h>
#include <map>
#include "../lib/Communicator/ICommunicator.h"
#include "../lib/Communicator/LoraCommunicator.h"
#include "../lib/Communicator/MockLoraCommunicator.h"
#include "../lib/Communicator/IridiumCommunicator.h"
#include "../lib/Communicator/MockIridiumCommunicator.h"
#include "../lib/Communicator/SerialCommunicator.h"
#include "../lib/CommunicatorRouter/CommunicatorRouter.h"
#include "../lib/Processor/PacketProcessor.h"
#include "../lib/Display/AdafruitDisplay.h"
#include "../lib/Display/SerialUSBDisplay.h"
#include "../lib/Routines/PingRoutine.h"
#include "../lib/RoutingTable/RoutingTable.h"

// FIXME: Must define OperationMode class to manage the state of the system

// Instancias de comunicadores
LoraCommunicator &loraCommunicator = LoraCommunicator::getInstance(defaultLoraConfig);
MockLoraCommunicator mockLoraCommunicator;
IridiumCommunicator iridiumCommunicator;
MockIridiumCommunicator mockIridiumCommunicator;
SerialCommunicator serialCommunicator(&mySerial3, 9600); // Serial1 is the hardware serial port PINS 0 and 1

// Instancia de la pantalla
AdafruitDisplay adafruitDisplay(&adafruit_SSD1306);
SerialUSBDisplay serialUSBDisplay;

// Instancias de rutinas
PingRoutine pingRoutine;
std::map<uint8_t, IRoutine *> serviceRoutines = {
    {Packet::OpCode::PING, &pingRoutine} // Mapear el código de operación 0x02 a la rutina PingRoutine
};

// FIXME: Must define specific routes for lora and iridium packets
std::map<uint8_t, ICommunicator*> localizerRoutes = {
    {(Packet::Address::BACKEND | Packet::Address::LORA_PACKET), &serialCommunicator},
    {(Packet::Address::BACKEND | Packet::Address::IRIDIUM_PACKET), &serialCommunicator},
    {(Packet::Address::DRIFTER | Packet::Address::LORA_PACKET), &mockLoraCommunicator},
    {(Packet::Address::PI3 | Packet::Address::LORA_PACKET), &mockLoraCommunicator},
    {(Packet::Address::DRIFTER | Packet::Address::IRIDIUM_PACKET), &mockIridiumCommunicator},
    {(Packet::Address::PI3 | Packet::Address::IRIDIUM_PACKET), &mockIridiumCommunicator}};
RoutingTable localizerRoutingTable(localizerRoutes);

std::map<uint8_t, ICommunicator *> drifterRoutes = {
    {Packet::Address::BACKEND, &loraCommunicator},
    {Packet::Address::LOCALIZER, &loraCommunicator},
    {Packet::Address::PI3, &serialCommunicator}};
RoutingTable drifterRoutingTable(drifterRoutes);

// Instancia del procesador de mensajes
PacketProcessor messageProcessor(&serialUSBDisplay, serviceRoutines);

// Instancia del puente serial
auto localizerRouter = CommunicatorRouter(Packet::Address::LOCALIZER,
                                          &localizerRoutingTable,
                                          &messageProcessor,
                                          {&serialCommunicator, &mockLoraCommunicator, &mockIridiumCommunicator});

auto drifterRouter = CommunicatorRouter(Packet::Address::DRIFTER,
                                        &drifterRoutingTable,
                                        &messageProcessor,
                                        {&serialCommunicator, &loraCommunicator, &iridiumCommunicator});

void setup()
{
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

    // Inicializa el comunicador Serial
    serialCommunicator.init();

    // Inicializa el comunicador LoRa
    // loraCommunicator.init();

    // Inicializa el comunicador Iridium
    // iridiumCommunicator.init();

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

    // Reenviar mensajes desde el puerto serial al comunicador y viceversa
    localizerRouter.operate();
}

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void SERCOM3_Handler()
{
    mySerial3.IrqHandler();
}