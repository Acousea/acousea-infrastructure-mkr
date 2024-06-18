#ifndef SERIAL_BRIDGE_H
#define SERIAL_BRIDGE_H

#include <Arduino.h>
#include <vector>
#include "../Communicator/ICommunicator.h"
#include "../Processor/IProcessor.h"

// Clase principal que actúa como puente
class SerialBridge {
private:
    ICommunicator* communicator;
    IProcessor* processor;
    const uint8_t syncByte = 0x20;
    const uint8_t mask = 0b01000000;

public:
    SerialBridge(ICommunicator* communicator, IProcessor* processor) 
        : communicator(communicator), processor(processor) {}

    void relayFromSerial() {
        if (Serial.available() >= 4) {
            uint8_t sync = Serial.read();
            if (sync != syncByte) return;            

            uint8_t address = Serial.read();
            uint8_t codOp = Serial.read();
            uint8_t length = Serial.read();

            uint8_t payload[length];
            Serial.readBytes(payload, length);

            // Construir el mensaje completo
            uint8_t message[4 + length] = {sync, address, codOp, length};
            memcpy(message + 4, payload, length);

            
            if (address == Address::PI3 || address == Address::DRIFTER) { // Si la dirección es 01 o 11, el mensaje debe ser reenviado al comunicador
                communicator->send(message, sizeof(message));
            } else {
                processor->processMessage(message, 4 + length);
            }
        }
    }

    void relayToSerial() {
        while (communicator->available()) {
            std::vector<uint8_t> packet = communicator->read();
            uint8_t sync = packet[0];
            if (sync != syncByte) return;

            uint8_t address = packet[1];
            
            if (address == Address::PI3 || address == Address::DRIFTER) { // Si la dirección es 01 o 11, el mensaje debe ser reenviado al comunicador
                Serial.write(packet.data(), packet.size());
            } else {
                processor->processMessage(packet.data(), packet.size());
            }
        }
    }
};

#endif