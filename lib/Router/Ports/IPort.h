#ifndef IPORT_H
#define IPORT_H


#include <vector>
#include <deque>
#include <cstdint>
#include <string>

// Definición de la interfaz de comunicador
class IPort {
public:
    enum class PortType {
        LoraPort,
        SBDPort,
        SerialPort
    };

    explicit IPort(PortType type) : type(type) {
    }

    [[nodiscard]] PortType getType() const {
        return type;
    }

    [[nodiscard]] std::string getPortTypeString() const {
        switch (type) {
            case PortType::LoraPort: return "LoRa";
            case PortType::SBDPort: return "SBD";
            case PortType::SerialPort: return "Serial";
            default: return "Unknown";
        }
    }

    [[nodiscard]] static std::string portTypeToString(PortType type) {
        switch (type) {
            case PortType::LoraPort: return "LoraPort";
            case PortType::SBDPort: return "SBDPort";
            case PortType::SerialPort: return "SerialPort";
            default: return "Unknown";
        }
    }

public:
    virtual void init() = 0;

    virtual void send(const std::vector<uint8_t> &data) = 0; // Cambiado para aceptar datos crudos
    virtual bool available() = 0;

    virtual std::vector<std::vector<uint8_t> > read() = 0; // Devuelve lista de vectores de bytes


protected:
    PortType type;
    std::deque<std::vector<uint8_t>> receivedRawPackets;
    static const size_t MAX_QUEUE_SIZE = 10;
    static const size_t MAX_RECEIVED_PACKET_SIZE = 340;
};

#endif // IPORT_H
