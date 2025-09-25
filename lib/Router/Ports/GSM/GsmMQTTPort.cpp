#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)
#include "GsmMQTTPort.hpp"

GsmMQTTPort* GsmMQTTPort::instance = nullptr;

unsigned long GsmMQTTPort::getTime(){
    // get the current time from the GSM module
    return instance->gsmAccess.getTime();
}

void GsmMQTTPort::mqttMessageHandler(int messageSize){
    std::vector<uint8_t> packet;
    packet.reserve(messageSize);

    while (instance->mqttClient.available()){
        const char c = static_cast<char>(instance->mqttClient.read());
        packet.push_back(static_cast<uint8_t>(c));
    }

    if (!packet.empty()){
        if (instance->receivedRawPackets.size() >= GsmMQTTPort::MAX_QUEUE_SIZE){
            instance->receivedRawPackets.pop_front();
        }
        instance->receivedRawPackets.push_back(packet);
    }
}

GsmMQTTPort::GsmMQTTPort(const GsmConfig& cfg)
// : IPort(PortType::GsmPort), config(cfg), sslClient(gsmClient), mqttClient(sslClient){
    : IPort(PortType::GsmPort), config(cfg), mqttClient(ublox_gsmSslClient){
}

void GsmMQTTPort::printCertificates(const std::vector<StoredCert>& currentCerts){
    if (currentCerts.empty()){
        Logger::logWarning(getClassNameString() + " -> There are no existing certificates.");
    }
    else{
        Logger::logInfo(
            getClassNameString() + " -> Found " + std::to_string(currentCerts.size()) + " existing certificates:"
        );
    }
    for (const auto& cert : currentCerts){
        Logger::logWarning(
            getClassNameString() + " -> Existing Cert - Type: " + cert.type +
            ", Name: " + cert.internalName +
            ", Expiration: " + cert.expiration);
    }
}

void GsmMQTTPort::init(){
    Logger::logFreeMemory(getClassNameString() + " -> Initializing...");

    instance = this; // Store the active instance

    Logger::logInfo(getClassNameString() + " -> Connecting to GSM network...");
    while ((gsmAccess.begin(config.pin) != GSM_READY) ||
        (gprs.attachGPRS(config.apn, config.user, config.pass) != GPRS_READY)){
        Logger::logWarning(getClassNameString() + " -> GSM/GPRS not available, retrying...");
        delay(2000);
    }
    Logger::logInfo(getClassNameString() + " -> GSM/GPRS connected");

    // ========= Initialize SSL/TLS ==========
    // ublox_gsmSslClient.setModemDebug();
    // auto result = ublox_gsmSslClient.removeTrustedRootCertificates(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    // if (result){
    //     Logger::logInfo(getClassNameString() + " -> Default trusted root CAs removed successfully.");
    // }
    // else{
    //     Logger::logWarning(getClassNameString() + " -> No default trusted root CAs to remove or error occurred.");
    // }
    Logger::logFreeMemory(getClassNameString() + " -> Pre-loading root CAs...");
    ublox_gsmSslClient.setUserRoots(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    // ublox_gsmSslClient.importTrustedRoots(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    // ublox_gsmSslClient.setTrustedRoots(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);

    Logger::logFreeMemory(getClassNameString() + " -> Pre-Listing current certificates...");
    {
        const std::vector<StoredCert> currentCerts = ublox_gsmSslClient.listCertificates(CertType::All);
        GsmMQTTPort::printCertificates(currentCerts);
        Logger::logFreeMemory(getClassNameString() + " -> Post-Listing current certificates...");
    }
    Logger::logFreeMemory(getClassNameString() + " -> Post-Bracket current certificates...");


    // ublox_gsmSslClient.setModemNoDebug();

    // --------------------------- TRUST ANCHORS ---------------------------
    // ublox_gsmSslClient.updateCerts(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    Logger::logInfo(getClassNameString() + " -> Loading device private key...");
    ublox_gsmSslClient.setPrivateKey(private_key_pkcs1_pem, "my_device_key", private_key_pkcs1_pem_len);

    Logger::logInfo(getClassNameString() + " -> Loading device certificate...");
    ublox_gsmSslClient.setSignedCertificate(certificate_mkr1400_pem, "my_device_cert", certificate_mkr1400_pem_len);

    Logger::logInfo(getClassNameString() + " -> Using device private key for TLS...");
    ublox_gsmSslClient.usePrivateKey("my_device_key");

    Logger::logInfo(getClassNameString() + " -> Using device certificate for TLS...");
    ublox_gsmSslClient.useSignedCertificate("my_device_cert");

    // ublox_gsmSslClient.setModemNoDebug();

    Logger::logInfo(getClassNameString() +
        " -> Credentials loaded: Certificate 'device_cert', Private Key 'device_key'"
    );

    Logger::logFreeMemory(getClassNameString() + " -> Post-initializing GSM SSL client...");

    // ========== Initialize MQTT broker ==========
    mqttClient.onMessage(GsmMQTTPort::mqttMessageHandler);
    mqttClient.setId(config.clientId);

    Logger::logInfo(getClassNameString() + " -> Connecting to MQTT broker...");
    while (!mqttClient.connect(config.broker, config.port)
    ){
        Logger::logWarning(getClassNameString() + " -> MQTT connection failed, retrying...");
        delay(5000);
    }
    Logger::logInfo(getClassNameString() + " -> Successfully connected to MQTT broker");

    // Automatically subscribe to input topic
    mqttSubscribeToTopic(config.getInputTopic().c_str());

    Logger::logInfo(getClassNameString() + " -> Finished initialization.");
}


void GsmMQTTPort::send(const std::vector<uint8_t>& data){
    const std::string topic = config.getOutputTopic();
    mqttPublishToTopic(data, topic);
}

bool GsmMQTTPort::available(){
    return !receivedRawPackets.empty();
}

std::vector<std::vector<uint8_t>> GsmMQTTPort::read(){
    mqttLoop();
    std::vector<std::vector<uint8_t>> packets(receivedRawPackets.begin(), receivedRawPackets.end());
    receivedRawPackets.clear();
    return packets;
}

bool GsmMQTTPort::mqttPublishToTopic(const std::vector<uint8_t>& data, const std::string topic){
    const std::string prefix = getClassNameString() + " -> ";

    if (!mqttClient.beginMessage(topic.c_str())){
        Logger::logError(prefix + "Failed to begin MQTT message on topic " + topic);
        return true;
    }

    const size_t written = mqttClient.write(data.data(), data.size());
    if (written != data.size()){
        Logger::logWarning(prefix + "Partial write while publishing to " + topic +
            " (expected " + std::to_string(data.size()) +
            " bytes, wrote " + std::to_string(written) + ")");
    }

    if (!mqttClient.endMessage()){
        Logger::logError(prefix + "Failed to finalize MQTT message to " + topic +
            " (only wrote " + std::to_string(written) + " bytes from " +
            std::to_string(data.size()) + ")");
        return true;
    }

    Logger::logInfo(prefix + "Published " + std::to_string(written) +
        " bytes to topic " + topic);
    return false;
}


void GsmMQTTPort::mqttSubscribeToTopic(const char* topic){
    const std::string prefix = getClassNameString() + " -> ";
    const int result = mqttClient.subscribe(topic);

    switch (result){
    case 0:
        Logger::logError(prefix + "Failed to subscribe to topic: " + std::string(topic));
        break;

    case 1:
        Logger::logInfo(prefix + "Successfully subscribed to topic " + std::string(topic) + " with QoS 0");
        break;

    case 2:
        Logger::logInfo(prefix + "Successfully subscribed to topic " + std::string(topic) + " with QoS 1");
        break;

    case 3:
        Logger::logInfo(prefix + "Successfully subscribed to topic " + std::string(topic) + " with QoS 2");
        break;

    default:
        Logger::logWarning(prefix + "Unexpected subscribe return code (" +
            std::to_string(result) + ") for topic " + std::string(topic));
        break;
    }
}


void GsmMQTTPort::mqttLoop(){
    mqttClient.poll();
}


void GsmMQTTPort::testConnection(const char* host, int port, const char* path){
    Logger::logInfo(
        getClassNameString() + " -> Testing connection to " + std::string(host) + ":" + std::to_string(port));

    Logger::logInfo(getClassNameString() + " -> Using TLS (ublox_gsmSslClient)");
    if (!ublox_gsmSslClient.connect(host, port)){
        Logger::logError(getClassNameString() + " -> TLS connection FAILED");
        return;
    }

    Logger::logInfo(getClassNameString() + " -> TLS connection SUCCESS");

    // Example: send a simple HTTP GET
    ublox_gsmSslClient.print(String("GET ") + path + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Connection: close\r\n\r\n");

    // Read response
    std::string response;
    while (ublox_gsmSslClient.connected()){
        while (ublox_gsmSslClient.available()){
            char c = ublox_gsmSslClient.read();
            response += c;
        }
    }
    ublox_gsmSslClient.stop();
    Logger::logInfo(getClassNameString() + " -> Response:\n" + response);
}

#endif // ARDUINO && PLATFORM_HAS_GSM
