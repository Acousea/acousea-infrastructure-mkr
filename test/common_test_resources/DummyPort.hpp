#ifndef ACOUSEA_INFRASTRUCTURE_MKR_MOCKPORT_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_MOCKPORT_HPP

#include "Ports/IPort.h"
#include <vector>


// ======================================================================
// Mock simple de puerto para pruebas
// ======================================================================
class DummyPort : public IPort
{
public:
    explicit DummyPort(PortType t) : IPort(t)
    {
    }

    void init() override
    {
    }

    bool send(const std::vector<uint8_t>& data) override
    {
        sentPackets.push_back(data);
        return sendReturn;
    }

    bool available() override { return !inQueue.empty(); }

    std::vector<std::vector<uint8_t>> read() override
    {
        auto tmp = inQueue;
        inQueue.clear();
        return tmp;
    }

    void enqueueRaw(const std::vector<uint8_t>& raw) { inQueue.push_back(raw); }
    void setSendReturn(bool val) { sendReturn = val; }

    std::vector<std::vector<uint8_t>> sentPackets;

private:
    std::vector<std::vector<uint8_t>> inQueue;
    bool sendReturn{true};
};

#endif //ACOUSEA_INFRASTRUCTURE_MKR_MOCKPORT_HPP
