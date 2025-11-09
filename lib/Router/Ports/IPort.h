#ifndef IPORT_H
#define IPORT_H


#include <vector>
#include <deque>
#include <cstdint>

// Definición de la interfaz de comunicador
class IPort
{
public:
    enum class PortType
    {
        None,
#ifdef PLATFORM_HAS_GSM
        GsmMqttPort,
#endif
#ifdef PLATFORM_HAS_LORA
        LoraPort,
#endif
        SBDPort,
        SerialPort,
    };

    explicit IPort(PortType type) : type(type)
    {
    }

    [[nodiscard]] PortType getType() const
    {
        return type;
    }

    [[nodiscard]] static constexpr const char* portTypeToCString(const PortType type)
    {
        switch (type)
        {
        case PortType::None: return "None";
#ifdef PLATFORM_HAS_GSM
        case PortType::GsmMqttPort: return "GsmMqttPort";
#endif
#ifdef PLATFORM_HAS_LORA
        case PortType::LoraPort: return "LoraPort";
#endif
        case PortType::SBDPort: return "SBDPort";
        case PortType::SerialPort: return "SerialPort";
        default: return "Unknown";
        }
    }

public:
    virtual void init() = 0;

    virtual bool send(const std::vector<uint8_t>& data) = 0; // Cambiado para aceptar datos crudos
    virtual bool available() = 0;

    virtual std::vector<std::vector<uint8_t>> read() = 0; // Devuelve lista de vectores de bytes


protected:
    ~IPort() = default;
    PortType type;
    std::deque<std::vector<uint8_t>> receivedRawPackets;
    static constexpr size_t MAX_QUEUE_SIZE = 10;
    static constexpr size_t MAX_RECEIVED_PACKET_SIZE = 340;
};

#endif // IPORT_H
