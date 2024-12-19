#include "LoRaPort.h"

LoRaConfig defaultLoraConfig = {
        7,       // bandwidth_index: 125 kHz
        10,      // spreadingFactor: 7
        5,       // codingRate: 5
        2,       // txPower: 14 dBm
        (long) 868E6,   // frequency: 868 MHz
        8,       // preambleLength: 8
        0x12     // syncWord: 0x12
};

double bandwidth_kHz[10] = {
        7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, 500E3
};

LoraPort::LoraPort(const LoRaConfig &config) : IPort(PortType::LoraPort),
                                               config(config) {}

void LoraPort::init() {
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

void LoraPort::send(const Packet &packet) {
    SerialUSB.println("LORA_PORT::send() -> Sending packet...");
    SerialUSB.println("Packet: " + String(packet.encode().c_str()));
    while (!LoRa.beginPacket()) {
        SerialUSB.println("LORA_PORT::send() -> LoRa.beginPacket() Waiting for transmission to end...");
        delay(10);
    }
    auto packetBytes = packet.toBytes();
    LoRa.write(packetBytes.data(), packetBytes.size());
    LoRa.endPacket(); // Transmit the packet synchrously (blocking) -> Avoids setting onTxDone callback (has bugs in the library)
    // Start listening for incoming packets again
    LoRa.receive();
}

bool LoraPort::available() {
    return receivedPackets.size() > 0;
}

Result<Packet> LoraPort::read() {
    if (!available()) {
        return Result<Packet>::failure("No packets available");
    }
    Packet receivedPacket = receivedPackets.front(); // Acceder al primer elemento
    receivedPackets.pop_front(); // Eliminar el primer elemento
    return Result<Packet>::success(receivedPacket);
}

void LoraPort::onReceive(int packetSize) {
    if (packetSize == 0) return;

    std::vector<uint8_t> buffer;
    while (LoRa.available()) {
        buffer.push_back(LoRa.read());
    }
    Packet packet = Packet::fromBytes(buffer);
    if (receivedPackets.size() >= MAX_QUEUE_SIZE) {
        // Emitir advertencia y eliminar el paquete más antiguo
        SerialUSB.println("WARNING: Received packet queue is full. Dropping the oldest packet.");
        receivedPackets.pop_front(); // Eliminar el paquete más antiguo
    }

    SerialUSB.println("Correctly received packet -> pushing");
    receivedPackets.push_back(packet);
}

void LoraPort::configureLora(const LoRaConfig &config) {
    LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidth_index]));
    LoRa.setSpreadingFactor(config.spreadingFactor);
    LoRa.setCodingRate4(config.codingRate);
    LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);
}