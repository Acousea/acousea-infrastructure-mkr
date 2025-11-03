#ifndef MOCK_IRIDIUM_PORT_H
#define MOCK_IRIDIUM_PORT_H


#include "ClassName.h"
#include "Ports/IPort.h"

class MockIridiumPort : public IPort
{
    CLASS_NAME(MockIridiumPort)

public:
    MockIridiumPort();

public:
    void init() override;

    bool send(const std::vector<uint8_t>& data) override;

    bool available() override;

    std::vector<std::vector<uint8_t>> read() override;
};

#endif // MOCK_IRIDIUM_PORT_H
