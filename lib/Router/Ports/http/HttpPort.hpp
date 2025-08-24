#ifndef HTTPPORT_HPP
#define HTTPPORT_HPP

#ifdef __linux__
#include "Ports/IPort.h"

class HttpPort : public IPort
{
public:
    HttpPort(std::string rxUrl, std::string txUrl, long timeoutMs);

    void init() override;

    void send(const std::vector<uint8_t>& data) override;

    bool available() override;

    std::vector<std::vector<uint8_t>> read() override;

private:
    bool fetchOne(); // intenta traer 1 mensaje y guardarlo en receivedRawPackets

    std::string rxUrl;
    std::string txUrl;
    long timeoutMs;
    bool initialized = false;
};

#endif // __linux__

#endif //HTTPPORT_HPP
