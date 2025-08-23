#ifdef ARDUINO
#include "IridiumPort.h"

#include <Logger/Logger.h>

Uart mySerial3(&sercom3, SBD_RX_PIN, SBD_TX_PIN, SERCOM_RX_PAD_1, UART_TX_PAD_0);

// Global instance of the IridiumSBD modem
IridiumSBD sbd_modem(IridiumSerial, SBD_SLEEP_PIN, SBD_RING_PIN);

#define DIAGNOSTICS 1
#if DIAGNOSTICS

void ISBDConsoleCallback(IridiumSBD *device, char c) {
    Serial.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c) {
    Serial.write(c);
}

#endif

IridiumPort::IridiumPort() : IPort(PortType::SBDPort) {
}

void IridiumPort::init() {
    // Set the pins to use mySerial3
    pinPeripheral(0, PIO_SERCOM); // Assign TX function to pin 0
    pinPeripheral(1, PIO_SERCOM); // Assign RX function to pin 1

    /** Uncomment this block to manually set the pins (wakeup modem again). Pins managed by the library by default
     * // Ring Indicator signal (active low)
     * pinMode(SBD_RING_PIN, INPUT_PULLUP);
     *
     * // Set the sleep pin to HIGH to wake up the modem (Sleep control (pull to ground to switch off))
     * pinMode(SBD_SLEEP_PIN, OUTPUT);
     * digitalWrite(SBD_SLEEP_PIN, HIGH);
    */
    sbd_modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);

    // According to the documentation this should go before calling begin()
    sbd_modem.enableRingAlerts(true);

    IridiumSerial.begin(SBD_MODEM_BAUDS);
    Logger::logInfo("IridiumPort::init() -> Initializing Iridium modem...");
    if (const int err = sbd_modem.begin(); err != ISBD_SUCCESS) {
        handleError(err);
    }
    sbd_modem.enableRingAlerts(true); // Documentations says this should go before begin(), but not sure if it works
    Logger::logInfo("IridiumPort::init() -> Iridium modem initialized");
}

void IridiumPort::send(const std::vector<uint8_t> &data) {
    Logger::logInfo("IridiumPort::send() -> Sending packet... " + Logger::vectorToHexString(data));

    uint8_t rxBuffer[MAX_RECEIVED_PACKET_SIZE];
    size_t rxBufferSize = sizeof(rxBuffer);
    const int err = sbd_modem.sendReceiveSBDBinary(data.data(), data.size(), rxBuffer, rxBufferSize);
    if (err != ISBD_SUCCESS) {
        handleError(err);
        return;
    }
    // Store received packet if available
    if (rxBufferSize > 0) {
        storeReceivedPacket(rxBuffer, rxBufferSize);
    }

    // Check for additional waiting messages if any
    if (sbd_modem.getWaitingMessageCount() > 0) {
        receiveIncomingMessages();
    }
}

bool IridiumPort::available() {
    // checkSignalQuality();
    checkRingAlertsAndWaitingMsgCount();
    // receiveIncomingMessages();
    return !receivedRawPackets.empty();
}

std::vector<std::vector<uint8_t> > IridiumPort::read() {
    std::vector<std::vector<uint8_t> > packets = {};
    for (const auto &packet: receivedRawPackets) {
        packets.push_back(packet);
    }
    receivedRawPackets.clear();
    return packets;
}


void IridiumPort::handleError(const int err) {
    std::string errorMessage = "IridiumPort::handleError(): " + std::to_string(err) + " - ";
    switch (err) {
        case ISBD_SUCCESS:
            errorMessage += "Success";
            break;
        case ISBD_ALREADY_AWAKE:
            errorMessage += "Already awake";
            break;
        case ISBD_SERIAL_FAILURE:
            errorMessage += "Serial failure";
            break;
        case ISBD_PROTOCOL_ERROR:
            errorMessage += "Protocol failure";
            break;
        case ISBD_CANCELLED:
            errorMessage += "Cancelled";
            break;
        case ISBD_NO_MODEM_DETECTED:
            errorMessage += "No modem detected";
            break;
        case ISBD_SBDIX_FATAL_ERROR:
            errorMessage += "SBDIX fatal failure";
            break;
        case ISBD_SENDRECEIVE_TIMEOUT:
            errorMessage += "Send/receive timeout";
            break;
        case ISBD_RX_OVERFLOW:
            errorMessage += "RX overflow";
            break;
        case ISBD_REENTRANT:
            errorMessage += "Reentrant";
            break;
        case ISBD_IS_ASLEEP:
            errorMessage += "Is asleep";
            break;
        case ISBD_NO_SLEEP_PIN:
            errorMessage += "No sleep pin";
            break;
        case ISBD_NO_NETWORK:
            errorMessage += "No network";
            break;
        case ISBD_MSG_TOO_LONG:
            errorMessage += "Message too long";
            break;
        default:
            errorMessage += "Unknown failure";
            break;
    }
    Logger::logError(errorMessage);
}

void IridiumPort::storeReceivedPacket(const uint8_t *data, size_t length) {
    // Print the received data
    const auto packet = std::vector<uint8_t>(data, data + length);
    Logger::logInfo("IridiumPort::storeReceivedPacket() -> Received data: " + Logger::vectorToHexString(packet));
    if (receivedRawPackets.size() >= MAX_QUEUE_SIZE) {
        Logger::logInfo("IridiumPort::storeReceivedPacket() -> Received packet queue is full. Dropping the oldest packet.");
        receivedRawPackets.pop_front();
    }
    receivedRawPackets.push_back(packet);
}

void IridiumPort::receiveIncomingMessages() {
    Logger::logInfo("IridiumPort::receiveIncomingMessages() -> Checking for incoming messages...");
    uint8_t rxBuffer[MAX_RECEIVED_PACKET_SIZE];
    do {
        size_t rxBufferSize = sizeof(rxBuffer);
        if (const int err = sbd_modem.sendReceiveSBDBinary(NULL, 0, rxBuffer, rxBufferSize); err != ISBD_SUCCESS) {
            handleError(err);
            break;
        }
        if (rxBufferSize > 0) {
            storeReceivedPacket(rxBuffer, rxBufferSize);
        } else {
            Logger::logInfo("IridiumPort::receiveIncomingMessages() -> No data read.");
        }
    } while (sbd_modem.getWaitingMessageCount() > 0);
}

void IridiumPort::checkRingAlertsAndWaitingMsgCount() {
    const bool ringAlert = sbd_modem.hasRingAsserted();
    const int incomingMessages = sbd_modem.getWaitingMessageCount();

    Logger::logInfo("IridiumPort::checkRingAlerts() -> Ring alert: " + std::to_string(ringAlert) +
                    ", Incoming messages: " + std::to_string(incomingMessages));

    if (ringAlert || incomingMessages > 0) {
        Logger::logInfo("IridiumPort::checkRingAlerts() -> Checking for incoming messages.");
        receiveIncomingMessages();
    }
}

void IridiumPort::checkSignalQuality() {
    Logger::logInfo("IridiumPort::checkSignalQuality() -> Checking signal quality...");
    int signalQuality = -1;
    if (const int err = sbd_modem.getSignalQuality(signalQuality); err != ISBD_SUCCESS) {
        handleError(err);
        return;
    }
    Logger::logInfo("IridiumPort::checkSignalQuality() -> Signal quality: " + std::to_string(signalQuality));
}

#endif // ARDUINO