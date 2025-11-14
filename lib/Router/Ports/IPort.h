#ifndef IPORT_H
#define IPORT_H


#include <cstdint>
#include <cstddef>


// Definición de la interfaz de comunicador
class IPort
{
public:
    enum class PortType: uint8_t
    {
        None = 0,
#ifdef PLATFORM_HAS_GSM
        GsmMqttPort = 1,
#endif
#ifdef PLATFORM_HAS_LORA
        LoraPort=2,
#endif
        SBDPort = 3,
        SerialPort = 4,
    };

    static constexpr uint8_t MAX_PORT_TYPE_U8 = static_cast<uint8_t>(PortType::SerialPort);

    explicit IPort(
        const PortType type)
        : type(type)
    {
    }

    [[nodiscard]] PortType getTypeEnum() const
    {
        return type;
    }

    [[nodiscard]] uint8_t getTypeU8() const
    {
        return static_cast<uint8_t>(type);
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

    virtual bool send(const uint8_t* data, size_t length) = 0;

    virtual bool available() = 0;

    virtual uint16_t readInto(uint8_t* buffer, uint16_t maxSize) = 0; // Devuelve lista de vectores de bytes

    virtual bool sync() = 0;


protected:
    ~IPort() = default;
    PortType type;
};

#endif // IPORT_H
