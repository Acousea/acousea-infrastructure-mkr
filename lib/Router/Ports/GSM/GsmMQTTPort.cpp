#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

// #define RESET_GSM_ON_INIT

#include "GsmMQTTPort.hpp"
#include <ErrorHandler/ErrorHandler.h>
// #include "../../private_keys/cert.h"
// #include "../../private_keys/key.h"

GsmMQTTPort* GsmMQTTPort::instance = nullptr;

unsigned long GsmMQTTPort::getTime()
{
    // get the current time from the GSM module
    return instance->ublox_gsmSslClient.getTime();
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
    : IPort(PortType::GsmMqttPort),
      config(cfg),
      mqttClient(ublox_gsmSslClient)
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
    LOG_FREE_MEMORY("%s -> Initializing...", getClassNameCString());

    // Store the active instance
    instance = this;

    // ========= Set debug mode for modem ==========
    UBlox201_GSMSSLClient::setModemDebug();

    // ========= Initialize SSL/TLS ==========
    ublox_gsmSslClient.init(config);

    // #define RESET_GSM_ON_INIT
#ifdef RESET_GSM_ON_INIT
    LOG_CLASS_INFO(" -> Resetting GSM modem...");
    {
        if (const auto resultOK = UBlox201_GSMSSLClient::removeCustomTrustedRoots(); resultOK)
            LOG_CLASS_INFO(" -> Custom trusted root CAs removed successfully.");
        else
            LOG_CLASS_WARNING(" -> No custom trusted root CAs to remove or error occurred.");
        if (const auto resultOK = UBlox201_GSMSSLClient::removeDefaultTrustedRoots(); resultOK)
            LOG_CLASS_INFO(" -> Default trusted root CAs removed successfully.");
        else
            LOG_CLASS_WARNING(" -> No default trusted root CAs to remove or error occurred.");

        UBlox201_GSMSSLClient::importTrustedRoots(GSM_CUSTOM_ROOT_CERTS, GSM_CUSTOM_NUM_ROOT_CERTS);
        ublox_gsmSslClient.setTrustedRoots(GSM_CUSTOM_ROOT_CERTS, GSM_CUSTOM_NUM_ROOT_CERTS);

        LOG_FREE_MEMORY("%s -> Post-loading root CAs...", getClassNameCString());

        LOG_FREE_MEMORY("%s -> Pre-Listing current certificates...", getClassNameCString());
        {
            const std::vector<StoredCert> currentCerts = UBlox201_GSMSSLClient::listCertificates(CertType::All);
            // GsmMQTTPort::printCertificates(currentCerts);
            LOG_FREE_MEMORY("%s ->  Post-Listing current certificates...", getClassNameCString());
        }
        LOG_FREE_MEMORY("%s -> Post-Bracket current certificates...", getClassNameCString());


        LOG_CLASS_INFO(" -> Removing existing device credentials (if any)...");
        const auto removePkeyOk = UBlox201_GSMSSLClient::removePrivateKey("my_device_key");
        removePkeyOk
            ? LOG_CLASS_INFO(" -> Existing device private key removed.")
            : LOG_CLASS_INFO(" -> No existing device private key to remove or error occurred.");

        LOG_CLASS_INFO(" -> Loading device private key...");
        // ublox_gsmSslClient.setPrivateKey(private_key_pkcs1_pem, "my_device_key", private_key_pkcs1_pem_len);
        ublox_gsmSslClient.setPrivateKey(reinterpret_cast<const uint8_t*>(config.privateKey),
                                         "my_device_key",
                                         strlen(config.privateKey));

        LOG_CLASS_INFO(" -> Removing existing device certificate (if any)...");
        const auto removeCertOk = UBlox201_GSMSSLClient::removeClientCertificate("my_device_cert");
        removeCertOk
            ? LOG_CLASS_INFO(" -> Existing device certificate removed.")
            : LOG_CLASS_INFO(" -> No existing device certificate to remove or error occurred.");

        LOG_CLASS_INFO(" -> Loading device certificate...");
        // ublox_gsmSslClient.setSignedCertificate(certificate_mkr1400_pem, "my_device_cert", certificate_mkr1400_pem_len);
        ublox_gsmSslClient.setSignedCertificate(reinterpret_cast<const uint8_t*>(config.certificate),
                                                "my_device_cert",
                                                strlen(config.certificate));
    }
#endif


    // --------------------------- DEVICE CERTIFICATE & PRIVATE KEY ---------------------------
    const std::vector<StoredCert> currentCerts = UBlox201_GSMSSLClient::listCertificates(CertType::All);
    GsmMQTTPort::printCertificates(currentCerts);

    LOG_CLASS_INFO(" -> Using Amazon Root CA for TLS...");
    ublox_gsmSslClient.setTrustedRoot(GSM_CUSTOM_ROOT_CERTS[0].name);

    LOG_CLASS_INFO(" -> Using device private key for TLS...");
    ublox_gsmSslClient.usePrivateKey("my_device_key");

    LOG_CLASS_INFO(" -> Using device certificate for TLS...");
    ublox_gsmSslClient.useSignedCertificate("my_device_cert");

    UBlox201_GSMSSLClient::printTLSProfileStatus();

    UBlox201_GSMSSLClient::setModemNoDebug();

    LOG_CLASS_INFO(" -> Credentials loaded: Certificate 'device_cert', Private Key 'device_key'");

    // String modemTime;
    // LOG_CLASS_INFO(" -> Retrieving time from modem...");
    // const bool clockOk = ublox_gsmSslClient.getClock(modemTime);
    // clockOk
    //     ? LOG_CLASS_INFO(" -> Modem time retrieved: %s", modemTime.c_str())
    //     : LOG_CLASS_WARNING(" -> Failed to retrieve modem time.");

    LOG_FREE_MEMORY("%s -> Post-initializing GSM SSL client...", getClassNameCString());


    // ========== Initialize MQTT broker ==========
    mqttClient.onMessage(GsmMQTTPort::mqttMessageHandler);
    mqttClient.setId(config.clientId);

    LOG_CLASS_INFO(" -> Connecting to MQTT broker %s:%d... Modem Time is %lu",
                   config.broker, config.port, getTime()
    );

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
        LOG_CLASS_WARNING("Partial write while publishing to %s (expected %d bytes, wrote %d bytes)",
                          topic, size, written);
    }


    if (!mqttClient.endMessage())
    {
        LOG_CLASS_ERROR("Failed to finalize MQTT message on topic %s (only wrote %d bytes from %d)",
                        topic, written, size);
        return true;
    }


    LOG_CLASS_INFO("Published %d bytes to %s", written, topic);
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
