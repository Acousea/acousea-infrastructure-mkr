#ifndef MOCK_LORA_PORT_H
#define MOCK_LORA_PORT_H


#include "ClassName.h"
#include "Ports/IPort.h"

class MockLoRaPort : public IPort {
    CLASS_NAME(MockLoRaPort)
public:
    MockLoRaPort();

public:

    void init() override;

    bool send(const std::vector<uint8_t>& data) override;

    bool available() override;

    std::vector<std::vector<uint8_t>> read() override;

};


#endif // MOCK_LORA_PORT_H
