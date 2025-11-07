#ifndef GSM_PORT_H
#define GSM_PORT_H

#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)


#include "UBlox201/UBlox201_GSMSSLClient.hpp" // UBlox201 GSM SSL Client. OVERRIDES internal GSMSSLClient configuration, MUST BE INCLUDED BEFORE <MKRGSM.h>

#include <MKRGSM.h>
#include <ArduinoMqttClient.h>
#include <Logger/Logger.h>
#include <ErrorHandler/ErrorHandler.h>

#include "GsmConfig.hpp"

#include "Ports/IPort.h"
#include "ClassName.h"

class GsmMQTTPort final : public IPort{
    CLASS_NAME(GsmPort)

public:
    explicit GsmMQTTPort(const GsmConfig& cfg);

    static void printCertificates(const std::vector<StoredCert>& currentCerts);

    void init() override;
    bool send(const std::vector<uint8_t>& data) override;
    bool available() override;
    std::vector<std::vector<uint8_t>> read() override;

    // Métodos MQTT
    void mqttLoop();
    void mqttSubscribeToTopic(const char* topic);
    bool mqttPublishToTopic(const uint8_t* data, size_t size, const char* topic);

private:
    static unsigned long getTime();
    static void mqttMessageHandler(int messageSize);

private:
    GsmConfig config;

    GSM gsmAccess;
    GPRS gprs;

    UBlox201_GSMSSLClient ublox_gsmSslClient;
    MqttClient mqttClient; // Cliente MQTT moderno (ArduinoMqttClient)

private:
    static constexpr char HEXMAP[] = "0123456789ABCDEF";
    static GsmMQTTPort* instance; // puntero a la instancia activa
};


#endif // ARDUINO && PLATFORM_HAS_GSM

#endif // GSM_PORT_HPP
