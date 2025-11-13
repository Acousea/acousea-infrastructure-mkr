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
    [[nodiscard]] bool clear(uint8_t port);
    bool isEmpty() const;
    bool arePortsEmpty(const uint8_t* ports, size_t portCount) const;
    [[nodiscard]] bool isPortEmpty(uint8_t port) const;

    [[nodiscard]] bool push(uint8_t port, const uint8_t* data, uint16_t length);
    [[nodiscard]] uint16_t popNext(uint8_t port, uint8_t* outBuffer, uint16_t maxOutSize);
    [[nodiscard]] uint16_t popAny(uint8_t* outBuffer, uint16_t maxOutSize);
    uint16_t popAnyFromPorts(const uint8_t* ports, size_t portCount, uint8_t* outBuffer, uint16_t maxOutSize);

private:
    StorageManager& storage_;
    RTCController& rtc_;
    const char* queueBaseName_ = SD_PATH("/queue");
    static constexpr uint8_t MAX_PORT = static_cast<uint8_t>(IPort::MAX_PORT_TYPE_U8);

    uint64_t writeOffset_[MAX_PORT + 1]; // Current write offsets for each port (1-based index)
    uint64_t readOffset_[MAX_PORT + 1];  // Current read offsets for each port (1-based index)
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_PACKETQUEUE_HPP
