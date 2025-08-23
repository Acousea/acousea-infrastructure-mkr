#ifndef MOCK_LORA_PORT_H
#define MOCK_LORA_PORT_H


#include "Ports/IPort.h"

class MockLoRaPort : public IPort {
public:
    MockLoRaPort();

public:

    void init() override;

    void send(const std::vector<uint8_t> &data) override;

    bool available() override;

    std::vector<std::vector<uint8_t>> read() override;

};


#endif // MOCK_LORA_PORT_H
