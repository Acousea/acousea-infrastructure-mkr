#ifndef GSM_PORT_H
#define GSM_PORT_H

#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)


#include "UBlox201/UBlox201_GSMSSLClient.hpp" // UBlox201 GSM SSL Client. OVERRIDES internal GSMSSLClient configuration, MUST BE INCLUDED BEFORE <MKRGSM.h>



#include <ArduinoMqttClient.h>
#include <Logger/Logger.h>
#include "GsmConfig.hpp"
#include "Ports/IPort.h"
#include "ClassName.h"
#include "PacketQueue/PacketQueue.hpp"

/**
 * @brief MQTT over GSM/TLS client for AWS IoT Core.
 *
 * This class manages a secure MQTT connection (TLS 1.2) to AWS IoT Core using u-blox GSM modems.
 * It complies with the AWS IoT Core MQTT 3.1.1 specification and enforces the following key concepts:
 *
 * - **Quality of Service (QoS):**
 *   - QoS 0 → messages are sent once, no acknowledgment (best-effort delivery).
 *   - QoS 1 → messages are delivered *at least once*; AWS confirms each with PUBACK.
 *   - AWS IoT Core does **not** support QoS 2.
 *
 * - **Persistent sessions:**
 *   - When `cleanSession = false`, AWS stores QoS 1 messages and subscriptions for up to 1 hour.
 *   - On reconnection, the device resumes the same session and receives queued messages.
 *
 * - **ACK and retry limits:**
 *   - Max **100 unacknowledged QoS 1** messages per client (inbound and outbound).
 *   - Each pending message expires after **960 seconds (16 minutes)** if unacknowledged.
 *   - Persistent session data (stored while disconnected) expires after **3600 seconds (1 hour)**.
 *
 * - **Recommended parameters for GSM links:**
 *   - KeepAlive interval: 60 seconds
 *   - Clean session: false (to enable persistence)
 *   - QoS: 1 for reliable telemetry
 *
 * For more details, see:
 * - AWS IoT MQTT specification: https://docs.aws.amazon.com/iot/latest/developerguide/mqtt.html
 * - Service quotas and message broker limits: https://docs.aws.amazon.com/general/latest/gr/iot-core.html#message-broker-limits
 */

class GsmMQTTPort final : public IPort
{
    CLASS_NAME(GsmPort)

public:
    explicit GsmMQTTPort(const GsmConfig& cfg, PacketQueue& packetQueue);

    static void printCertificates(const std::vector<StoredCert>& currentCerts);
    bool tryConnect();

    void init() override;
    bool send(const uint8_t* data, size_t length) override;
    bool available() override;

    // Métodos MQTT
    bool sync() override;

    void mqttStop();

private:
    void mqttSubscribeToTopic(const char* topic);
    bool mqttPublishToTopic(const uint8_t* data, size_t size, const char* topic, bool retained, uint8_t qos);
    bool mqttPublishToTopic(const char* payload, const char* topic, bool retained, uint8_t qos);
    void setupLastWill();
    static unsigned long getTime();
    static void mqttMessageHandler(int messageSize);

private:
    GsmConfig config;
    UBlox201_GSMSSLClient ublox_gsmSslClient;
    MqttClient mqttClient; // Cliente MQTT moderno (ArduinoMqttClient)
    PacketQueue& packetQueue_;
    static constexpr char HEXMAP[] = "0123456789ABCDEF";
    static GsmMQTTPort* instance; // puntero a la instancia activa
    static constexpr char MQTT_DISCONNECT_MESSAGE[] = R"({"state":"offline"})";
    static constexpr char MQTT_CONNECT_MESSAGE[] = R"({"state":"online"})";
};


#endif // ARDUINO && PLATFORM_HAS_GSM

#endif // GSM_PORT_HPP
