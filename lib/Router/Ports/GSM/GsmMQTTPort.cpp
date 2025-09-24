#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)
#include "GsmMQTTPort.hpp"

GsmMQTTPort* GsmMQTTPort::instance = nullptr;

GsmMQTTPort::GsmMQTTPort(const GsmConfig& cfg)
    : IPort(PortType::GsmPort), config(cfg), sslClient(gsmClient), mqttClient(sslClient){
}

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

void GsmMQTTPort::init(){
    Logger::logInfo(getClassNameString() + " -> Initializing...");

    instance = this; // Store the active instance

    // ========== Initialize ECCX08 ==========
    if (!ECCX08.begin()){
        ErrorHandler::handleError(getClassNameString() + " -> No ECCX08 detected");
        return;
    }

    // ========== Initialize GSM/GPRS ==========
    ArduinoBearSSL.onGetTime(GsmMQTTPort::getTime);
    sslClient.setEccSlot(0, config.certificate);
    Logger::logInfo(getClassNameString() + " -> Connecting to GSM network...");
    while ((gsmAccess.begin(config.pin) != GSM_READY) ||
        (gprs.attachGPRS(config.apn, config.user, config.pass) != GPRS_READY)){
        Logger::logWarning(getClassNameString() + " -> GSM/GPRS not available, retrying...");
        delay(2000);
    }
    Logger::logInfo(getClassNameString() + " -> GSM/GPRS connected");


    // ========== Initialize MQTT broker ==========

    mqttClient.onMessage(GsmMQTTPort::mqttMessageHandler);
    mqttClient.setId(config.clientId);

    Logger::logInfo(getClassNameString() + " -> Connecting to MQTT broker...");
    while (!mqttClient.connect(config.broker, config.port)){
        Logger::logWarning(getClassNameString() + " -> MQTT connection failed, retrying...");
        delay(5000);
    }
    Logger::logInfo(getClassNameString() + " -> Successfully connected to MQTT broker");

    // Automatically subscribe to input topic
    subscribeToTopic(config.getInputTopic().c_str());
}


void GsmMQTTPort::send(const std::vector<uint8_t>& data){
    // Publish to the output topic
    mqttClient.beginMessage(config.getOutputTopic().c_str());
    for (const uint8_t b : data){
        mqttClient.write(b);
    }
    mqttClient.endMessage();

    Logger::logInfo(getClassNameString() + " -> Message published to topic " + config.getOutputTopic());
}


bool GsmMQTTPort::available(){
    return !receivedRawPackets.empty();
}

std::vector<std::vector<uint8_t>> GsmMQTTPort::read(){
    std::vector<std::vector<uint8_t>> packets(receivedRawPackets.begin(), receivedRawPackets.end());
    receivedRawPackets.clear();
    return packets;
}


void GsmMQTTPort::subscribeToTopic(const char* topic){
    mqttClient.subscribe(topic);
    Logger::logInfo(getClassNameString() + " -> Subscribed to topic: " + std::string(topic));
}

void GsmMQTTPort::publishToTopic(const char* topic, const char* message){
    mqttClient.beginMessage(topic);
    mqttClient.print(message);
    mqttClient.endMessage();
    Logger::logInfo(getClassNameString() + " -> Message published to topic " + std::string(topic));
}

void GsmMQTTPort::mqttLoop(){
    mqttClient.poll();
}


void GsmMQTTPort::testConnection(const char* host, int port, const char* path, bool useSSL){
    Logger::logInfo(
        getClassNameString() + " -> Testing connection to " + std::string(host) + ":" + std::to_string(port));

    if (useSSL){
        Logger::logInfo(getClassNameString() + " -> Using TLS (BearSSLClient)");
        if (!sslClient.connect(host, port)){
            Logger::logError(getClassNameString() + " -> TLS connection FAILED");
            return;
        }

        Logger::logInfo(getClassNameString() + " -> TLS connection SUCCESS");

        // Example: send a simple HTTP GET
        sslClient.print(String("GET ") + path + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Connection: close\r\n\r\n");

        // Read response
        while (sslClient.connected()){
            while (sslClient.available()){
                char c = sslClient.read();
                Serial.print(c);
            }
        }
        sslClient.stop();
    }
    else{
        Logger::logInfo(getClassNameString() + " -> Using plain TCP (GSMSSLClient)");
        if (!gsmClient.connect(host, port)){
            Logger::logError(getClassNameString() + " -> TCP connection FAILED");
            return;
        }
        Logger::logInfo(getClassNameString() + " -> TCP connection SUCCESS");
        gsmClient.stop();
    }
}

#endif // ARDUINO && PLATFORM_HAS_GSM
