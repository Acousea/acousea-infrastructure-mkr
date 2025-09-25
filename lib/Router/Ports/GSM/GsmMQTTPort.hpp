#ifndef GSM_PORT_H
#define GSM_PORT_H

#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

#include <deque>
#include <string>

#include <MKRGSM.h>

#include <ArduinoMqttClient.h>
#include <Logger/Logger.h>
#include <ErrorHandler/ErrorHandler.h>

#include "UBlox201/UBlox201_GSMSSLClient.hpp"
#include "Ports/IPort.h"
#include "ClassName.h"

#include "../../private_keys/cert.h"
#include "../../private_keys/key.h"

struct GsmConfig{
    const char* pin; // PIN de la SIM ("" si no tiene)
    const char* apn; // APN del operador
    const char* user; // Usuario APN
    const char* pass; // Password APN
    const char* clientId; // Identificador cliente MQTT (ej: IMEI o ECCX08 SN)
    const char* broker; // Endpoint AWS IoT
    int port; // Puerto destino, normalmente 8883
    const char* certificate; // Certificado cliente en PEM


    static constexpr auto baseTopic = "acousea/nodes"; // prefijo común fijo

    // Devuelve el topic de entrada
    [[nodiscard]] std::string getInputTopic() const{
        return std::string(baseTopic) + "/" + clientId + "/in";
    }

    // Devuelve el topic de salida
    [[nodiscard]] std::string getOutputTopic() const{
        return std::string(baseTopic) + "/" + clientId + "/out";
    }
};


class GsmMQTTPort final : public IPort{
    CLASS_NAME(GsmPort)

public:
    explicit GsmMQTTPort(const GsmConfig& cfg);
    static void printCertificates(const std::vector<StoredCert>& currentCerts);

    void init() override;
    void send(const std::vector<uint8_t>& data) override;
    bool available() override;
    std::vector<std::vector<uint8_t>> read() override;

    // Métodos MQTT
    void subscribeToTopic(const char* topic);
    void publishToTopic(const char* topic, const char* message);
    void mqttLoop();
    void testConnection(const char* host, int port, const char* path);

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
