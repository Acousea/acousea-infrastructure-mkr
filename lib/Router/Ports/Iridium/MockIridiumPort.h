#ifndef MOCK_IRIDIUM_PORT_H
#define MOCK_IRIDIUM_PORT_H


#include "Ports/IPort.h"

class MockIridiumPort : public IPort {

public:
    MockIridiumPort();

public:
    void init() override;

    void send(const Packet &packet) override;

    bool available() override;

    Result<Packet> read() override;
};

#endif // MOCK_IRIDIUM_PORT_H