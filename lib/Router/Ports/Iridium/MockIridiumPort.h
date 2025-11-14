#ifndef MOCK_IRIDIUM_PORT_H
#define MOCK_IRIDIUM_PORT_H


#include "ClassName.h"
#include "Ports/IPort.h"

class MockIridiumPort final : public IPort
{
    CLASS_NAME(MockIridiumPort)

public:

    explicit MockIridiumPort();

    void init() override;

    bool send(const uint8_t* data, size_t length) override;

    bool available() override;

    uint16_t readInto(uint8_t* buffer, uint16_t maxSize) override;

    bool sync() override;

};

#endif // MOCK_IRIDIUM_PORT_H
