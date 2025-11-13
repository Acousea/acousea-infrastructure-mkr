#if defined(PLATFORM_ARDUINO) && defined(PLATFORM_HAS_LORA)
#include "LoRaPort.h"

#include <ErrorHandler/ErrorHandler.h>
#include <Logger/Logger.h>

LoRaConfig defaultLoraConfig = {
    7, // bandwidth_index: 125 kHz
    10, // spreadingFactor: 7
    5, // codingRate: 5
    2, // txPower: 14 dBm
    (long)868E6, // frequency: 868 MHz
    8, // preambleLength: 8
    0x12 // syncWord: 0x12
};

double bandwidth_kHz[10] = {
    7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, 500E3
};

LoraPort::LoraPort(FlashPacketQueue& flashQueue, const LoRaConfig& config) :
    IPort(PortType::LoraPort, flashQueue),
    config(config)
{
}

void LoraPort::init()
{
    configureLora(config);
    LoRa.setSyncWord(config.syncWord);
    LoRa.setPreambleLength(config.preambleLength);

    if (!LoRa.begin(config.frequency))
    {
        ERROR_HANDLE_CLASS("LoRaPort::init() -> Failed to initialize LoRa module");
        return;
    }

    LoRa.onReceive(onReceiveWrapper);
    LoRa.receive(); // Start listening for incoming packets
}

bool LoraPort::send(const uint8_t* data, size_t length)
{
    LOG_CLASS_INFO("LoraPort::send() -> Sending packet... %s",
                   Logger::vectorToHexString(data, length).c_str());

    while (!LoRa.beginPacket())
    {
        LOG_CLASS_INFO("LORA_PORT::send() -> LoRa.beginPacket() Waiting for transmission to end...");
        delay(10);
    }
    LoRa.write(data, length);
    LoRa.endPacket();
    // Transmit the packet synchrously (blocking) -> Avoids setting onTxDone callback (has bugs in the library)
    // Start listening for incoming packets again
    LoRa.receive();
    return true;
}

bool LoraPort::available()
{
    return !flashPacketQueue_.isEmptyForPort(getTypeU8());
}

uint16_t LoraPort::readInto(uint8_t* buffer, const uint16_t maxSize)
{
    return flashPacketQueue_.popForPort(getTypeU8(), buffer, maxSize);
}

bool LoraPort::sync()
{
    return true;
}


void LoraPort::onReceive(int packetSize)
{
    if (packetSize == 0) return;

    std::vector<uint8_t> buffer;
    while (LoRa.available())
    {
        buffer.push_back(LoRa.read());
    }

    if (receivedRawPackets.size() >= MAX_QUEUE_SIZE)
    {
        // Emitir advertencia y eliminar el paquete más antiguo
        LOG_CLASS_INFO("LoRaPort::onReceive(): WARNING: Received packet queue is full. Dropping the oldest packet.");
        receivedRawPackets.pop_front(); // Eliminar el paquete más antiguo
    }

    LOG_CLASS_INFO("LoraPort::onReceive() -> Storing packet... %s",
                   Logger::vectorToHexString(buffer.data(), buffer.size()).c_str());
    receivedRawPackets.push_back(buffer);
}

void LoraPort::configureLora(const LoRaConfig& config)
{
    LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidth_index]));
    LoRa.setSpreadingFactor(config.spreadingFactor);
    LoRa.setCodingRate4(config.codingRate);
    LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);
}

#endif // ARDUINO
