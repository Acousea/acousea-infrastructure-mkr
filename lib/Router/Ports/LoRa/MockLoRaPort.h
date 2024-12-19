#ifndef MOCK_LORA_PORT_H
#define MOCK_LORA_PORT_H


#include "Ports/IPort.h"


class MockLoRaPort : public IPort {
public:
    MockLoRaPort();

public:

    void init() override;

    void send(const Packet &packet) override;

    bool available() override;

    Result<Packet> read() override;

};


#endif // MOCK_LORA_PORT_H
