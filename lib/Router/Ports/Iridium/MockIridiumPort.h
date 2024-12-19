#ifndef MOCK_IRIDIUM_PORT_H
#define MOCK_IRIDIUM_PORT_H


#include "Ports/IPort.h"
#include <Arduino.h>

class MockIridiumPort : public IPort {

public:
    MockIridiumPort();

public:
    void init() override;

    void send(const std::vector<uint8_t> &data) override;

    bool available() override;

    std::vector<std::vector<uint8_t>> read() override;
};

#endif // MOCK_IRIDIUM_PORT_H