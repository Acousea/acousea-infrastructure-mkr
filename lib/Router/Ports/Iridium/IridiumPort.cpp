#include "IridiumPort.h"

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

IridiumPort::IridiumPort() : IPort(PortType::SBDPort) {}

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
    SerialUSB.println("IridiumPort::init(): ");
    int err = sbd_modem.begin();
    if (err != ISBD_SUCCESS) {
        handleError(err);
    }
    sbd_modem.enableRingAlerts(true); // Documentations says this should go before begin(), but not sure if it works
    SerialUSB.println("IridiumPort::init(): Iridium modem initialized");
}

void IridiumPort::send(const std::vector<uint8_t> &data) {
    SerialUSB.println("IridiumPort::send() -> Sending packet...");
    SerialUSB.print("Data: ");
    for (const auto &byte: data) {
        SerialUSB.print(byte, HEX);
        SerialUSB.print(" ");
    }

    uint8_t rxBuffer[MAX_RECEIVED_PACKET_SIZE];
    size_t rxBufferSize = sizeof(rxBuffer);
    int err = sbd_modem.sendReceiveSBDBinary(data.data(),
                                             data.size(),
                                             rxBuffer,
                                             rxBufferSize);
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

std::vector<std::vector<uint8_t>> IridiumPort::read() {
    if (receivedRawPackets.empty()) {
        return {};
    }

    // Return all packets and clear the queue
    std::vector<std::vector<uint8_t>> packets;
    for (const auto &packet: receivedRawPackets) {
        packets.push_back(packet);
    }
    receivedRawPackets.clear();
    return packets;


}

void IridiumPort::handleError(int err) {
    SerialUSB.print("IridiumPort::handleError(): ");
    SerialUSB.println(err);
    switch (err) {
        case ISBD_SUCCESS:
            SerialUSB.println("Success");
            break;
        case ISBD_ALREADY_AWAKE:
            SerialUSB.println("Already awake");
            break;
        case ISBD_SERIAL_FAILURE:
            SerialUSB.println("Serial failure");
            break;
        case ISBD_PROTOCOL_ERROR:
            SerialUSB.println("Protocol failure");
            break;
        case ISBD_CANCELLED:
            SerialUSB.println("Cancelled");
            break;
        case ISBD_NO_MODEM_DETECTED:
            SerialUSB.println("No modem detected");
            break;
        case ISBD_SBDIX_FATAL_ERROR:
            SerialUSB.println("SBDIX fatal failure");
            break;
        case ISBD_SENDRECEIVE_TIMEOUT:
            SerialUSB.println("Send/receive timeout");
            break;
        case ISBD_RX_OVERFLOW:
            SerialUSB.println("RX overflow");
            break;
        case ISBD_REENTRANT:
            SerialUSB.println("Reentrant");
            break;
        case ISBD_IS_ASLEEP:
            SerialUSB.println("Is asleep");
            break;
        case ISBD_NO_SLEEP_PIN:
            SerialUSB.println("No sleep pin");
            break;
        case ISBD_NO_NETWORK:
            SerialUSB.println("No network");
            break;
        case ISBD_MSG_TOO_LONG:
            SerialUSB.println("Message too long");
            break;
        default:
            SerialUSB.println("Unknown failure");
            break;
    }
}

void IridiumPort::storeReceivedPacket(const uint8_t *data, size_t length) {
    // Print the received data
    SerialUSB.print("IridiumPort::storeReceivedPacket() -> Received data: ");
    for (size_t i = 0; i < length; i++) {
        SerialUSB.print(data[i], HEX);
        SerialUSB.print(" ");
    }
    SerialUSB.println();

    std::vector<uint8_t> packet = std::vector<uint8_t>(data, data + length);
    if (receivedRawPackets.size() >= MAX_QUEUE_SIZE) {
        SerialUSB.println("WARNING: Received packet queue is full. Dropping the oldest packet.");
        receivedRawPackets.pop_front();
    }
    receivedRawPackets.push_back(packet);
}

void IridiumPort::receiveIncomingMessages() {
    SerialUSB.println("IridiumPort::receiveIncomingMessages() -> Checking for incoming messages...");
    uint8_t rxBuffer[MAX_RECEIVED_PACKET_SIZE];
    size_t rxBufferSize;
    do {
        rxBufferSize = sizeof(rxBuffer);
        int err = sbd_modem.sendReceiveSBDBinary(NULL, 0, rxBuffer, rxBufferSize);
        if (err != ISBD_SUCCESS) {
            handleError(err);
            break;
        } else {
            if (rxBufferSize > 0) storeReceivedPacket(rxBuffer, rxBufferSize);
            else SerialUSB.println("IridiumPort::receiveIncomingMessages() -> No data read.");
        }
    } while (sbd_modem.getWaitingMessageCount() > 0);
}

void IridiumPort::checkRingAlertsAndWaitingMsgCount() {
    bool ringAlert = sbd_modem.hasRingAsserted();
    int incomingMessages = sbd_modem.getWaitingMessageCount();
    SerialUSB.println("IridiumPort::checkRingAlertsAndWaitingMsgCount() -> Ring alert: " + String(ringAlert) +
                      ", Incoming messages: " + String(incomingMessages));
    if (!ringAlert && incomingMessages <= 0) {
        return;
    }

    if (ringAlert) {
        SerialUSB.println("IridiumPort::checkRingAlerts() -> Ring alert detected. Checking for incoming messages.");
    }

    if (incomingMessages > 0) {
        SerialUSB.println(
                "IridiumPort::checkRingAlerts() -> Incoming messages count > 0. Checking for incoming messages.");
    }

    receiveIncomingMessages();
}

void IridiumPort::checkSignalQuality() {
    SerialUSB.println("IridiumPort::checkSignalQuality() -> Checking signal quality...");
    int signalQuality = -1;
    int err = sbd_modem.getSignalQuality(signalQuality);
    if (err != ISBD_SUCCESS) {
        handleError(err);
        return;
    }
    SerialUSB.print("Signal quality (0-5): ");
    SerialUSB.println(signalQuality);
}


