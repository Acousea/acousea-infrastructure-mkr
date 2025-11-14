#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

// #define RESET_GSM_ON_INIT

#include "GsmMQTTPort.hpp"
#include <ErrorHandler/ErrorHandler.h>
#include "SharedMemory/SharedMemory.hpp"

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
    LOG_CLASS_INFO(" -> MQTT Message Handler invoked. Message size: %d bytes", messageSize);
    auto packetBuffer = reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer());
    constexpr size_t maxBufferSize = SharedMemory::tmpBufferSize();
    size_t readCount = 0;


    while (instance->mqttClient.available())
    {
        const char c = static_cast<char>(instance->mqttClient.read());
        if (readCount >= maxBufferSize)
        {
            LOG_CLASS_WARNING(" -> MQTT Message Handler: Buffer overflow, message too large.");
            break;
        }
        packetBuffer[readCount++] = static_cast<uint8_t>(c);
    }

    LOG_CLASS_INFO(" -> MQTT Message Handler: Received %d bytes", readCount);
    if (readCount != static_cast<size_t>(messageSize))
    {
        LOG_CLASS_WARNING(" -> MQTT Message Handler: Warning - expected size %d but read %d bytes", messageSize,
                          readCount);
    }
    if (readCount > 0)
    {
        const bool pushOK = instance->packetQueue_.push(
            static_cast<uint8_t>(IPort::PortType::GsmMqttPort),
            packetBuffer,
            static_cast<uint16_t>(readCount)
        );
        if (!pushOK)
        {
            LOG_CLASS_ERROR(" -> MQTT Message Handler: Error storing packet in flash queue.");
            return;
        }
        LOG_CLASS_INFO(" -> MQTT Message Handler: Packet stored in flash queue.");
    }
}

GsmMQTTPort::GsmMQTTPort(const GsmConfig& cfg, PacketQueue& packetQueue)
// : IPort(PortType::GsmPort), config(cfg), sslClient(gsmClient), mqttClient(sslClient){
    : IPort(PortType::GsmMqttPort),
      config(cfg),
      mqttClient(ublox_gsmSslClient),
      packetQueue_(packetQueue)
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

bool GsmMQTTPort::tryConnect()
{
    if (instance == nullptr)
    {
        LOG_CLASS_ERROR(" -> GsmMQTTPort::tryConnect() called before initialization.");
        return false;
    }
    if (mqttClient.connected())
    {
        LOG_CLASS_INFO(" -> Already connected to MQTT broker");
        return true;
    }
    constexpr unsigned long MAX_MQTT_CONNECT_RETRIES = 3; // 30 seconds
    unsigned long attempt = 0;
    while (!mqttClient.connect(config.broker, config.port) && (attempt++ < MAX_MQTT_CONNECT_RETRIES))
    {
        LOG_CLASS_WARNING(" -> MQTT connection failed, retrying...");
        delay(5000);
    }
    LOG_CLASS_INFO(" -> Successfully connected to MQTT broker");
    return mqttClient.connected();
}

void GsmMQTTPort::init()
{
    LOG_FREE_MEMORY("%s -> Initializing...", getClassNameCString());

    // Store the active instance
    instance = this;

    // ========= Set debug mode for modem ==========
    // UBlox201_GSMSSLClient::setModemDebug();

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
    // const std::vector<StoredCert> currentCerts = UBlox201_GSMSSLClient::listCertificates(CertType::All);
    // GsmMQTTPort::printCertificates(currentCerts);

    // LOG_CLASS_INFO(" -> Using Amazon Root CA for TLS...");
    // ublox_gsmSslClient.setTrustedRoot(GSM_CUSTOM_ROOT_CERTS[0].name);

    LOG_CLASS_INFO(" -> Using device private key for TLS...");
    ublox_gsmSslClient.usePrivateKey("my_device_key");

    LOG_CLASS_INFO(" -> Using device certificate for TLS...");
    ublox_gsmSslClient.useSignedCertificate("my_device_cert");

    // UBlox201_GSMSSLClient::printTLSProfileStatus();

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
    mqttClient.setCleanSession(false); // Persisted session to receive messages while offline
    mqttClient.setKeepAliveInterval(60 * 1000L); // 60 seconds keep-alive
    mqttClient.setConnectionTimeout(30 * 1000L); // 30 seconds for connection timeout


    LOG_CLASS_INFO(" -> Connecting to MQTT broker %s:%d... Modem Time is %lu",
                   config.broker, config.port, getTime()
    );


    // Try to connect to the MQTT broker
    tryConnect();

    // Setup Last Will and Testament (LWT)
    // setupLastWill();
    // LOG_CLASS_INFO(" -> Last Will and Testament (LWT) configured.");

    // Automatically subscribe to input topic
    mqttSubscribeToTopic(config.inputTopic);
    // LOG_CLASS_INFO(" -> Subscribed to input topic: %s", config.inputTopic);

    // Publish online status: Status topic, retain=true, QoS=1 (retain the last status)
    mqttPublishToTopic(MQTT_CONNECT_MESSAGE,
                       config.statusTopic,
                       false,
                       1);

    LOG_CLASS_INFO(" -> Finished initialization.");
}


bool GsmMQTTPort::send(const uint8_t* data, const size_t length)
{
    return mqttPublishToTopic(data, length, config.outputTopic, false, 1);
}

bool GsmMQTTPort::available()
{
    return !packetQueue_.isPortEmpty(getTypeU8());
}


bool GsmMQTTPort::mqttPublishToTopic(const uint8_t* data, size_t size, const char* topic, const bool retained,
                                     const uint8_t qos)
{
    // Try to connect to the MQTT broker
    const bool connOk = tryConnect();
    if (!connOk)
    {
        LOG_CLASS_ERROR(" -> Cannot publish to MQTT topic %s: not connected to broker", topic);
        return false;
    }
    LOG_CLASS_INFO(" -> Publishing %d bytes to topic: %s (retain=%s, QoS=%d)",
                   size, topic, retained ? "true" : "false", qos);
    if (!mqttClient.beginMessage(topic, false, 1)) // QoS 1, retain =false
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

bool GsmMQTTPort::mqttPublishToTopic(const char* payload, const char* topic, const bool retained, const uint8_t qos)
{
    // Try to connect to the MQTT broker
    tryConnect();

    LOG_CLASS_INFO(" -> Preparing to publish payload=%s to topic=%s |(retain=%s, QoS=%d)",
                   payload, topic, retained ? "true" : "false", qos);

    if (!payload || !topic || strlen(topic) == 0)
    {
        LOG_CLASS_ERROR("Invalid MQTT publish parameters (null or empty)");
        return false;
    }

    if (!mqttClient.beginMessage(topic, retained, qos))
    {
        LOG_CLASS_ERROR("Failed to begin MQTT message on topic: %s", topic);
        return false;
    }

    const size_t expectedSize = strlen(payload);
    const size_t written = mqttClient.print(payload);

    if (written != expectedSize)
    {
        LOG_CLASS_WARNING(
            "Partial MQTT payload write on topic %s (expected %d bytes, wrote %d bytes)",
            topic, expectedSize, written
        );
    }

    if (!mqttClient.endMessage())
    {
        LOG_CLASS_ERROR("Failed to finalize MQTT message on topic: %s (only wrote %d of %d bytes)",
                        topic, written, expectedSize);
        return false;
    }

    LOG_CLASS_INFO("Published %d bytes to %s", written, topic);
    return true;
}


void GsmMQTTPort::mqttSubscribeToTopic(const char* topic)
{
    const int result = mqttClient.subscribe(topic, 1);

    if (result == 0)
    {
        LOG_CLASS_ERROR("Failed to subscribe to topic: %s", topic);
        return;
    }

    // Obtener el QoS confirmado por el broker
    if (const int qosGranted = mqttClient.subscribeQoS();
        qosGranted >= 0 && qosGranted <= 2)
    {
        LOG_CLASS_INFO("Successfully subscribed to topic %s with granted QoS %d", topic, qosGranted);
    }
    else
    {
        LOG_CLASS_WARNING("Subscription to %s succeeded, but invalid QoS response (%d)", topic, qosGranted);
    }
}


bool GsmMQTTPort::sync()
{
    if (instance == nullptr)
    {
        LOG_CLASS_WARNING(" -> GsmMQTTPort::sync() called before init(). Ignoring.");
        return false;
    }
    if (!instance->mqttClient.connected())
    {
        LOG_CLASS_WARNING(" -> MQTT client not connected. Attempting to reconnect...");
        if (!instance->tryConnect())
        {
            return false;
        }
    }
    instance->mqttClient.poll();
    return true;
}


void GsmMQTTPort::setupLastWill()
{
    LOG_CLASS_INFO(" -> Configuring Last Will and Testament (LWT)...");

    if (strlen(config.statusTopic) == 0) // statusTopic will never be null, no need to check for it
    {
        LOG_CLASS_WARNING(" -> Skipping LWT setup: status topic not defined");
        return;
    }

    // Begin LWT configuration (retain = true, QoS = 1)
    if (!mqttClient.beginWill(config.statusTopic, true, 1))
    {
        LOG_CLASS_ERROR(" -> Failed to begin LWT message setup on topic: %s", config.statusTopic);
        return;
    }

    // Payload = "offline"
    mqttClient.print(MQTT_DISCONNECT_MESSAGE);

    if (!mqttClient.endWill())
    {
        LOG_CLASS_ERROR(" -> Failed to finalize LWT message on topic: %s", config.statusTopic);
        return;
    }

    LOG_CLASS_INFO(" -> LWT configured successfully on topic: %s (retain=true, QoS=1)",
                   config.statusTopic);
}


void GsmMQTTPort::mqttStop()
{
    if (mqttClient.connected())
    {
        // Status topic, retain=true, QoS=1 (retain the last status)
        mqttPublishToTopic(MQTT_DISCONNECT_MESSAGE,
                           config.statusTopic,
                           true,
                           1);
        mqttClient.stop();
    }
}


#endif // ARDUINO && PLATFORM_HAS_GSM
