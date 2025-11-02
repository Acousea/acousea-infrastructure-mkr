#ifndef HTTPPORT_HPP
#define HTTPPORT_HPP


#include "Ports/IPort.h"

class HttpPort : public IPort {
public:
    // baseUrl: ej. "http://127.0.0.1:8000"
    // imei   : el IMEI del “módem” simulado
    explicit HttpPort(std::string baseUrl,
                      std::string imei,
                      long timeoutMs = 5000,
                      int pollMax = 1);

    void init() override;
    bool send(const std::vector<uint8_t>& data) override;     // MO -> /enqueue_mo
    bool available() override;                                 // intenta poll si cola vacía
    std::vector<std::vector<uint8_t>> read() override;         // drena cola (poll si vacío)

private:

    bool fetchOne();                                           // hace 1 GET /modem/poll...
    static std::string bytesToHex(const std::vector<uint8_t>& in);
    static bool hexToBytes(const std::string& hex, std::vector<uint8_t>& out);


    std::string baseUrl_;
    std::string imei_;
    long timeoutMs_{};
    int  pollMax_{};
    bool initialized_{false};
};


#endif //HTTPPORT_HPP
