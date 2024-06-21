#ifndef LORA_COMMUNICATOR_H
#define LORA_COMMUNICATOR_H

#include <Arduino.h>
#include <deque>
#include <LoRa.h>
#include "ICommunicator.h"
#include "Packet.h"




class LoraCommunicator : public ICommunicator {
public:
    static LoraCommunicator& getInstance(const LoRaConfig& config = defaultLoraConfig) {
        static LoraCommunicator instance(config);
        return instance;
    }

    void init() {
        configureLora(config);
        LoRa.setSyncWord(config.syncWord);
        LoRa.setPreambleLength(config.preambleLength);

        if (!LoRa.begin(config.frequency)) {
            SerialUSB.println("Starting LoRa failed!");
            while (1);
        }        

        LoRa.receive(); // Start listening for incoming packets                
    }

    void send(const Packet& packet) override {
        while(!LoRa.beginPacket()) {
            SerialUSB.println("LORA_COMMUNICATOR::send() -> LoRa.beginPacket() Waiting for transmission to end...");
            delay(10);
        }
        LoRa.write(packet.getFullPacket(), packet.getFullPacketLength());
        LoRa.endPacket(); // Transmit the packet synchrously (blocking) -> Avoids setting onTxDone callback (has bugs in the library)
        LoRa.receive(); // Start listening for incoming packets again
        // Print through serial monitor for debugging
        Serial.print("LORA_COMMUNICATOR: Sending packet: ");
        for (size_t i = 0; i < packet.getFullPacketLength(); i++) {
            Serial.print(packet.getFullPacket()[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

    }

    bool available() override {
        return receivedPackets.size() > 0;
    }

    Packet read() override {
        if (!available()) {            
            return Packet(nullptr, 0); // Invalid packet if no packets are available
        }
        Packet receivedPacket = receivedPackets.front(); // Acceder al primer elemento
        receivedPackets.pop_front(); // Eliminar el primer elemento
        return receivedPacket; 
    }

private:
    const LoRaConfig& config;      
    static LoraCommunicator* instance;

    std::deque<Packet> receivedPackets;
    static const size_t MAX_QUEUE_SIZE = 10;

    LoraCommunicator(const LoRaConfig& config) : config(config) {        
        instance = this;  // Establecer la instancia en el constructor
        LoRa.onReceive(onReceiveWrapper);
    }

    // Eliminar los métodos de copia para garantizar que no haya múltiples instancias
    LoraCommunicator(const LoraCommunicator&) = delete;
    LoraCommunicator& operator=(const LoraCommunicator&) = delete;

    static void onReceiveWrapper(int packetSize) {
        SerialUSB.println("OnReceiveWrapper Callback");
        if (instance) {
            SerialUSB.println("FOUND INSTANCE -> processing");
            instance->onReceive(packetSize);
        } else {
          SerialUSB.println("NO INSTANCE!");
        }
    }

    void onReceive(int packetSize) {
        if (packetSize == 0) return;

        std::vector<uint8_t> buffer;
        while (LoRa.available()) {
            buffer.push_back(LoRa.read());
        }

        uint8_t* data = buffer.data();
        Packet packet(data, packetSize);
        if (!packet.isValid()) {
            SerialUSB.println("LORA_COMMUNICATOR::onReceive() -> Invalid packet received");
            return;
        }
        if (receivedPackets.size() >= MAX_QUEUE_SIZE) {
            // Emitir advertencia y eliminar el paquete más antiguo
            SerialUSB.println("WARNING: Received packet queue is full. Dropping the oldest packet.");
            receivedPackets.pop_front(); // Eliminar el paquete más antiguo
        }

        SerialUSB.println("Correctly received packet -> pushing");
        receivedPackets.push_back(packet);
    }


    void configureLora(const LoRaConfig& config) {
        LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidth_index]));
        LoRa.setSpreadingFactor(config.spreadingFactor);
        LoRa.setCodingRate4(config.codingRate);
        LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);
    }
};
// Inicializar la instancia estática
LoraCommunicator* LoraCommunicator::instance = nullptr;

#endif // LORA_COMMUNICATOR_H
