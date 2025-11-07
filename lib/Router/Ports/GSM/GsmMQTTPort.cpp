#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)
#include "GsmMQTTPort.hpp"
// #include "../../private_keys/cert.h"
// #include "../../private_keys/key.h"

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
#ifdef PLATFORM_ARDUINO
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
    // const auto result = ublox_gsmSslClient.removeTrustedRootCertificates(
    // reinterpret_cast<const GSMRootCert*>(GSM_ROOT_CERTS), GSM_NUM_ROOT_CERTS
    // );
    // result
    // ? LOG_CLASS_INFO(" -> Default trusted root CAs removed successfully.")
    // : LOG_CLASS_WARNING(" -> No default trusted root CAs to remove or error occurred.");

    // LOG_FREE_MEMORY("%s -> Pre-loading root CAs...", getClassNameCString());
    // ublox_gsmSslClient.setUserRoots(reinterpret_cast<const GSMRootCert*>(GSM_ROOT_CERTS), GSM_NUM_ROOT_CERTS);
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
    // ublox_gsmSslClient.setPrivateKey(private_key_pkcs1_pem, "my_device_key", private_key_pkcs1_pem_len);
    ublox_gsmSslClient.setPrivateKey(reinterpret_cast<const uint8_t*>(config.certificate),
                                     "my_device_key",
                                     strlen(config.certificate));

    LOG_CLASS_INFO(" -> Loading device certificate...");
    // ublox_gsmSslClient.setSignedCertificate(certificate_mkr1400_pem, "my_device_cert", certificate_mkr1400_pem_len);
    ublox_gsmSslClient.setSignedCertificate(reinterpret_cast<const uint8_t*>(config.privateKey),
                                            "my_device_cert",
                                            strlen(config.privateKey));

    // --------------------------- DEVICE CERTIFICATE & PRIVATE KEY ---------------------------


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


#endif // ARDUINO && PLATFORM_HAS_GSM
