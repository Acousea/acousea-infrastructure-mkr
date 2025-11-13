#ifndef MOCK_LORA_PORT_H
#define MOCK_LORA_PORT_H

#ifdef PLATFORM_HAS_LORA
#include "ClassName.h"
#include "Ports/IPort.h"

class MockLoRaPort : public IPort
{
    CLASS_NAME(MockLoRaPort)

public:
    explicit MockLoRaPort(FlashPacketQueue& packetQueue);

    void init() override;

    bool send(const uint8_t* data, size_t length) override;

    bool available() override;

    // Lee datos y construye un paquete
    uint16_t readInto(uint8_t* buffer, uint16_t maxSize) override;

    bool sync() override;
};

#endif // PLATFORM_HAS_LORA || UNIT_TESTING

#endif // MOCK_LORA_PORT_H
