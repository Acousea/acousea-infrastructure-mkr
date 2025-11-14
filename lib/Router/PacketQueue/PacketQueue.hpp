#ifndef ACOUSEA_INFRASTRUCTURE_MKR_PACKETQUEUE_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_PACKETQUEUE_HPP

#include <cstdint>

#include "ClassName.h"
#include "RTCController.hpp"
#include "Ports/IPort.h"
#include "StorageManager/StorageManager.hpp"
#include "StorageManager/SDStorageManager/SDPath/SDPath.hpp"


/**
 * @brief Clase que implementa una cola de paquetes persistente utilizando un StorageManager.
 */
class PacketQueue
{
    CLASS_NAME(PacketQueue)

public:
    static constexpr uint8_t START_BYTE = 0x7E;
    static constexpr uint8_t END_BYTE = 0x7F;

    static constexpr size_t HEADER_SIZE = 1 + 4 + 2; // start + timestamp(4) + length(2)
    static constexpr size_t FOOTER_SIZE = 1; // end

    explicit PacketQueue(StorageManager& storage, RTCController& rtc);

    [[nodiscard]] bool begin();

    // Clears all packets from the queue
    [[nodiscard]] bool clear(uint8_t port);

    // Checks if the queue is empty (no packets in any port)
    [[nodiscard]] bool isEmpty() const;

    // Checks if the specified ports are empty
    [[nodiscard]] bool arePortsEmpty(const uint8_t* ports, size_t portCount) const;

    // Checks if the specified port is empty
    [[nodiscard]] bool isPortEmpty(uint8_t port) const;

    // Pushes a packet into the queue for the specified port
    [[nodiscard]] bool push(uint8_t port, const uint8_t* data, uint16_t length);

    // Peeks the next packet from the specified port without removing it
    [[nodiscard]] uint16_t peekNext(uint8_t port, uint8_t* outBuffer, uint16_t maxOutSize);

    // Peeks any packet from the queue (FI-FO among all ports) without removing it
    [[nodiscard]] uint16_t peekAny(uint8_t* outBuffer, uint16_t maxOutSize);

    // Peeks any packet from the specified ports (FI-FO among them) without removing it
    [[nodiscard]] uint16_t peekAnyFromPorts(const uint8_t* ports, size_t portCount, uint8_t* outBuffer,
                                            uint16_t maxOutSize);

    // Pops the next packet from the specified port
    [[nodiscard]] uint16_t popNext(uint8_t port, uint8_t* outBuffer, uint16_t maxOutSize);

    // Pops any packet from the queue (FI-FO among all ports)
    [[nodiscard]] uint16_t popAny(uint8_t* outBuffer, uint16_t maxOutSize);

    // Pops any packet from the specified ports (FI-FO among them)
    [[nodiscard]] uint16_t popAnyFromPorts(const uint8_t* ports, size_t portCount, uint8_t* outBuffer,
                                           uint16_t maxOutSize);

    [[nodiscard]] bool skipToNextPacket(uint8_t port);

    [[nodiscard]] uint64_t getReadOffset(uint8_t port) const;

    [[nodiscard]] uint64_t getNextReadOffset(uint8_t port) const;

private:
    [[nodiscard]] uint16_t _next(uint8_t port, uint8_t* outBuffer, uint16_t maxOutSize, bool pop);

private:
    StorageManager& storage_;
    RTCController& rtc_;
    const char* queueBaseName_ = SD_PATH("/queue");
    static constexpr uint8_t MAX_PORT = static_cast<uint8_t>(IPort::MAX_PORT_TYPE_U8);

    uint64_t writeOffset_[MAX_PORT + 1]; // Current write offsets for each port (1-based index)
    uint64_t readOffset_[MAX_PORT + 1]; // Current read offsets for each port (1-based index)
    uint64_t nextReadOffset_[MAX_PORT + 1]; // Next read offsets for each port (1-based index)
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_PACKETQUEUE_HPP
