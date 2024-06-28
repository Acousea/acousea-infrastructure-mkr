#ifndef LORA_PORT_H
#define LORA_PORT_H

#include <Arduino.h>
#include <deque>
#include <LoRa.h>
#include "IPort.h"
#include "Packet.h"

typedef struct {
    uint8_t bandwidth_index;    // Ancho de banda [0,9]: Como norma general, multiplicarlo por dos supone reducir a la mitad el tiempo de transmisión.
                                // Reducir el ancho de banda permite reducir el impacto de interferencias y obtener distancias de enlace mayores.
    uint8_t spreadingFactor;    // Factor de dispersión [6,12]: Aumentar el Spf permite hacer que la señal aumente su inmunidad al ruido y puedan 
                                // lograrse distancias de enlace mayores. Sin embargo, el uso de factores de dispersión mayores incrementa de forma
                                // muy significativa el tiempo de transmisión de un dato, lo que puede provocar un aumento importante en el consumo
                                // de energía o exceder el tiempo actividad o duty-cycle permitido.
    uint8_t codingRate;         // Tasa de codificación [5, 8]:  Esta estrategia permite detectar y corregir, hasta cierto punto, los errores que 
                                // se produzcan en la recepción. Como contrapartida, aumentar la tasa de codificación incrementa el número de símbolos
                                // a transmitir y, por tanto, el tiempo de transmisión
    uint8_t txPower;            // Potencia de transmisión [2,20]: Potencia deseada expresada en dBm. . Para pruebas en interior es aconsejable poner 
                                // poner este valor al mínimo y evitar así saturar los receptores, ya que de lo contrario fallarán las transmisiones.
    long frequency;             // Frecuencia de operación
    uint8_t preambleLength;     // Longitud del preámbulo
    uint8_t syncWord;           // Palabra de sincronización
} LoRaConfig;

// Valores de ancho de banda en kHz
static double bandwidth_kHz[10] = {
    7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, 500E3
};


LoRaConfig defaultLoraConfig = {
    7,       // bandwidth_index: 125 kHz
    10,      // spreadingFactor: 7
    5,       // codingRate: 5
    2,       // txPower: 14 dBm
    (long) 868E6,   // frequency: 868 MHz
    8,       // preambleLength: 8
    0x12     // syncWord: 0x12
};

// Prototipo del callback
void onReceiveWrapper(int packetSize);

class LoraPort : public IPort {
public:
    LoraPort(const LoRaConfig& config = defaultLoraConfig) : config(config) {}

    void init() override {
        configureLora(config);
        LoRa.setSyncWord(config.syncWord);
        LoRa.setPreambleLength(config.preambleLength);

        if (!LoRa.begin(config.frequency)) {
            SerialUSB.println("Starting LoRa failed!");
            while (1);
        }        

        LoRa.onReceive(onReceiveWrapper);
        LoRa.receive(); // Start listening for incoming packets                
    }

    void send(const Packet& packet) override {
        while(!LoRa.beginPacket()) {
            SerialUSB.println("LORA_PORT::send() -> LoRa.beginPacket() Waiting for transmission to end...");
            delay(10);
        }
        LoRa.write(packet.getFullPacket(), packet.getFullPacketLength());
        LoRa.endPacket(); // Transmit the packet synchrously (blocking) -> Avoids setting onTxDone callback (has bugs in the library)
        // Start listening for incoming packets again
        LoRa.receive(); 
        
        packet.print(); 
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

    void onReceive(int packetSize) {
        if (packetSize == 0) return;

        std::vector<uint8_t> buffer;
        while (LoRa.available()) {
            buffer.push_back(LoRa.read());
        }

        uint8_t* data = buffer.data();
        Packet packet(data, packetSize);
        if (!packet.isValid()) {
            SerialUSB.println("LORA_PORT::onReceive() -> Invalid packet received");
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

private:
    const LoRaConfig& config;      

    std::deque<Packet> receivedPackets;
    static const size_t MAX_QUEUE_SIZE = 10;

    void configureLora(const LoRaConfig& config) {
        LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidth_index]));
        LoRa.setSpreadingFactor(config.spreadingFactor);
        LoRa.setCodingRate4(config.codingRate);
        LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);
    }
};



#endif // LORA_PORT_H
