#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)
#include "GsmMQTTPort.hpp"

#include "UBlox201/TrustAnchors.h"
#include "../../private_keys/cert.h"
#include "../../private_keys/key.h"

GsmMQTTPort* GsmMQTTPort::instance = nullptr;

unsigned long GsmMQTTPort::getTime()
{
    // get the current time from the GSM module
    return instance->gsmAccess.getTime();
}

void GsmMQTTPort::mqttMessageHandler(int messageSize)
{
    std::vector<uint8_t> packet;
    packet.reserve(messageSize);

    while (instance->mqttClient.available())
    {
        const char c = static_cast<char>(instance->mqttClient.read());
        packet.push_back(static_cast<uint8_t>(c));
    }

    if (!packet.empty())
    {
        if (instance->receivedRawPackets.size() >= GsmMQTTPort::MAX_QUEUE_SIZE)
        {
            instance->receivedRawPackets.pop_front();
        }
        instance->receivedRawPackets.push_back(packet);
    }
}

GsmMQTTPort::GsmMQTTPort(const GsmConfig& cfg)
// : IPort(PortType::GsmPort), config(cfg), sslClient(gsmClient), mqttClient(sslClient){
    : IPort(PortType::GsmMqttPort), config(cfg), mqttClient(ublox_gsmSslClient)
{
}

void GsmMQTTPort::printCertificates(const std::vector<StoredCert>& currentCerts)
{
    if (currentCerts.empty())
    {
        LOG_CLASS_WARNING(" -> There are no existing certificates.");
    }
    else
    {
        LOG_CLASS_INFO(" -> Found %d existing certificates:", currentCerts.size());
    }
    for (const auto& cert : currentCerts)
    {
        LOG_CLASS_WARNING(
            "Existing Cert - Type: %s, Name: %s, Expiration: %s",
            cert.type,
            cert.internalName,
            cert.expiration
        );
    }
}

void GsmMQTTPort::init()
{
#ifdef ARDUINO
    LOG_FREE_MEMORY("%s -> Initializing...", getClassNameCString());
#endif


    instance = this; // Store the active instance


    LOG_CLASS_INFO(" -> Connecting to GSM network...");
    while ((gsmAccess.begin(config.pin) != GSM_READY) ||
        (gprs.attachGPRS(config.apn, config.user, config.pass) != GPRS_READY))
    {
        LOG_CLASS_WARNING(" -> GSM/GPRS not available, retrying...");
        delay(2000);
    }
    LOG_CLASS_INFO(" -> GSM/GPRS connected");

    // ========= Initialize SSL/TLS ==========
    // ublox_gsmSslClient.setModemDebug();
    // auto result = ublox_gsmSslClient.removeTrustedRootCertificates(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    // if (result){
    //      LOG_CLASS_INFO(" -> Default trusted root CAs removed successfully.");
    // }
    // else{
    //      LOG_CLASS_WARNING(" -> No default trusted root CAs to remove or error occurred.");
    // }
    LOG_FREE_MEMORY("%s -> Pre-loading root CAs...", getClassNameCString());
    ublox_gsmSslClient.setUserRoots(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    // ublox_gsmSslClient.importTrustedRoots(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    // ublox_gsmSslClient.setTrustedRoots(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);


    LOG_FREE_MEMORY("%s -> Pre-Listing current certificates...", getClassNameCString());
    {
        const std::vector<StoredCert> currentCerts = ublox_gsmSslClient.listCertificates(CertType::All);
        GsmMQTTPort::printCertificates(currentCerts);
        LOG_FREE_MEMORY("%s ->  Post-Listing current certificates...", getClassNameCString());
    }
    LOG_FREE_MEMORY("%s -> Post-Bracket current certificates...", getClassNameCString());


    // ublox_gsmSslClient.setModemNoDebug();

    // --------------------------- TRUST ANCHORS ---------------------------
    // ublox_gsmSslClient.updateCerts(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    LOG_CLASS_INFO(" -> Loading device private key...");
    ublox_gsmSslClient.setPrivateKey(private_key_pkcs1_pem, "my_device_key", private_key_pkcs1_pem_len);

    LOG_CLASS_INFO(" -> Loading device certificate...");
    ublox_gsmSslClient.setSignedCertificate(certificate_mkr1400_pem, "my_device_cert", certificate_mkr1400_pem_len);

    LOG_CLASS_INFO(" -> Using device private key for TLS...");
    ublox_gsmSslClient.usePrivateKey("my_device_key");

    LOG_CLASS_INFO(" -> Using device certificate for TLS...");
    ublox_gsmSslClient.useSignedCertificate("my_device_cert");

    // ublox_gsmSslClient.setModemNoDebug();

    LOG_CLASS_INFO(" -> Credentials loaded: Certificate 'device_cert', Private Key 'device_key'");

    LOG_FREE_MEMORY("%s -> Post-initializing GSM SSL client...", getClassNameCString());


    // ========== Initialize MQTT broker ==========
    mqttClient.onMessage(GsmMQTTPort::mqttMessageHandler);
    mqttClient.setId(config.clientId);

    LOG_CLASS_INFO(" -> Connecting to MQTT broker...");
    while (!mqttClient.connect(config.broker, config.port)
    )
    {
        LOG_CLASS_WARNING(" -> MQTT connection failed, retrying...");
        delay(5000);
    }
    LOG_CLASS_INFO(" -> Successfully connected to MQTT broker");

    // Automatically subscribe to input topic
    mqttSubscribeToTopic(config.inputTopic);

    LOG_CLASS_INFO(" -> Finished initialization.");
}


bool GsmMQTTPort::send(const std::vector<uint8_t>& data)
{
    return mqttPublishToTopic(data.data(), data.size(), config.outputTopic);
}

bool GsmMQTTPort::available()
{
    return !receivedRawPackets.empty();
}

std::vector<std::vector<uint8_t>> GsmMQTTPort::read()
{
    mqttLoop();
    std::vector<std::vector<uint8_t>> packets(receivedRawPackets.begin(), receivedRawPackets.end());
    receivedRawPackets.clear();
    return packets;
}


bool GsmMQTTPort::mqttPublishToTopic(const uint8_t* data, size_t size, const char* topic)
{
    if (!mqttClient.beginMessage(topic))
    {
        LOG_CLASS_ERROR("Failed to begin MQTT message on %s", topic);
        return false;
    }

    const size_t written = mqttClient.write(data, size);
    if (written != size)
    {
        LOG_CLASS_WARNING("Partial write while publishing to %s (expected %zu bytes, wrote %zu)",
                          topic, size, written);
    }


    if (!mqttClient.endMessage())
    {
        LOG_CLASS_ERROR("Failed to finalize MQTT message on topic %s (only wrote %zu bytes from %zu)",
                        topic, written, size);
        return true;
    }


    LOG_CLASS_INFO("Published %zu bytes to %s", written, topic);
    return true;
}


void GsmMQTTPort::mqttSubscribeToTopic(const char* topic)
{
    const int result = mqttClient.subscribe(topic);

    switch (result)
    {
    case 0:
        LOG_CLASS_ERROR("Failed to subscribe to topic: %s", topic);
        break;
    case 1:
        LOG_CLASS_INFO("Successfully subscribed to topic %s with QoS 0", topic);
        break;
    case 2:
        LOG_CLASS_INFO("Successfully subscribed to topic %s with QoS 1", topic);
        break;
    case 3:
        LOG_CLASS_INFO("Successfully subscribed to topic %s with QoS 2", topic);
        break;
    default:
        LOG_CLASS_WARNING("Unexpected subscribe return code (%d) for topic %s", result, topic);
        break;
    }
}


void GsmMQTTPort::mqttLoop()
{
    mqttClient.poll();
}


void GsmMQTTPort::testConnection(const char* host, int port, const char* path)
{
    LOG_CLASS_INFO(" -> Testing connection to %s:%d", host, port);

    LOG_CLASS_INFO(" -> Using TLS (ublox_gsmSslClient)");
    if (!ublox_gsmSslClient.connect(host, port))
    {
        LOG_CLASS_ERROR(" -> TLS connection FAILED");
        return;
    }

    LOG_CLASS_INFO(" -> TLS connection SUCCESS");

    // ----------- Construir y enviar petición HTTP -----------
    char request[256];
    std::snprintf(request, sizeof(request),
                  "GET %s HTTP/1.1\r\n"
                  "Host: %s\r\n"
                  "Connection: close\r\n\r\n",
                  path, host);

    ublox_gsmSslClient.print(request);
    LOG_CLASS_INFO(" -> HTTP request sent.");

    // ----------- Leer respuesta (máx. 1024 bytes) -----------
    constexpr size_t MAX_RESPONSE_SIZE = 1024;
    char response[MAX_RESPONSE_SIZE + 1]; // +1 para '\0'
    size_t totalRead = 0;

    LOG_CLASS_INFO(" -> Reading up to %zu bytes of response...", MAX_RESPONSE_SIZE);

    while (ublox_gsmSslClient.connected() && totalRead < MAX_RESPONSE_SIZE)
    {
        while (ublox_gsmSslClient.available() && totalRead < MAX_RESPONSE_SIZE)
        {
            int c = ublox_gsmSslClient.read();
            if (c < 0)
                continue;

            response[totalRead++] = static_cast<char>(c);
        }
    }

    // Terminar cadena correctamente
    response[totalRead] = '\0';

    // Si se truncó la respuesta, indicarlo
    if (totalRead >= MAX_RESPONSE_SIZE)
    {
        LOG_CLASS_WARNING(" -> Response truncated to %zu bytes", MAX_RESPONSE_SIZE);
    }

    ublox_gsmSslClient.stop();

    LOG_CLASS_INFO(" -> Connection closed. Response (%zu bytes):\n%.1024s", totalRead, response);
}

#endif // ARDUINO && PLATFORM_HAS_GSM
