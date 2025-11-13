#ifndef ACOUSEA_INFRASTRUCTURE_MKR_SDPACKETQUEUE_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_SDPACKETQUEUE_HPP

#ifdef PLATFORM_ARDUINO

#include "ClassName.h"
#include "PacketQueue/PacketQueue.hpp"

class SDPacketQueue final : public PacketQueue
{
    CLASS_NAME(SDPacketQueue)
public:
    explicit SDPacketQueue(const char* queuePath = "/queue.dat");

    bool begin() override;
    bool isEmpty() const override;
    bool isEmptyForPort(uint8_t targetPortType) override;
    bool push(uint8_t portType, const uint8_t* data, uint16_t length) override;
    uint16_t popAny(uint8_t* outBuffer, uint16_t maxSize) override;
    uint16_t popForPort(uint8_t targetPortType, uint8_t* outBuffer, uint16_t maxSize) override;
    void clear() override;

private:
    const char* queuePath_;
    uint32_t head_ = 0;
    uint32_t tail_ = 0;
    static constexpr auto HEADER_SIZE = 3;
};

#endif // PLATFORM_ARDUINO

#endif //ACOUSEA_INFRASTRUCTURE_MKR_SDPACKETQUEUE_HPP
