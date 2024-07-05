#ifndef IRIDIUMPORT_H
#define IRIDIUMPORT_H

#include <Arduino.h>
#include <deque>
#include <IridiumSBD.h>
#include "IPort.h"
#include "../Packet/NullPacket.h"

// Define the necessary hardware connections for the Iridium modem
#define SBD_SLEEP_PIN 2 // OUTPUT, pull to GND to switch off
#define SBD_RING_PIN 3 // INPUT, driven LOW when new messages are available
#define SBD_MODEM_BAUDS 19200

Uart mySerial3(&sercom3, 1, 0, SERCOM_RX_PAD_1, UART_TX_PAD_0);
#define IridiumSerial mySerial3

// Global instance of the IridiumSBD modem
IridiumSBD sbd_modem(IridiumSerial, SBD_SLEEP_PIN, SBD_RING_PIN);

class IridiumPort : public IPort {
public:
    void init() override {
        // Set the pins to use mySerial3
        pinPeripheral(0, PIO_SERCOM); // Assign TX function to pin 0
        pinPeripheral(1, PIO_SERCOM); // Assign RX function to pin 1
        pinMode(SBD_RING_PIN, INPUT);
        pinMode(SBD_SLEEP_PIN, OUTPUT);
        // digitalWrite(SBD_RING_PIN, LOW);
        digitalWrite(SBD_SLEEP_PIN, HIGH);
        sbd_modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
        sbd_modem.enableRingAlerts(true);

        IridiumSerial.begin(SBD_MODEM_BAUDS);
        SerialUSB.println("IridiumPort::init(): ");
        int err = sbd_modem.begin();        
        if (err != ISBD_SUCCESS) {
            handleError(err);
        }
        SerialUSB.println("IridiumPort::init(): Iridium modem initialized");
    }

    void send(const Packet& packet) override {
        SerialUSB.println("IridiumPort::send() -> Sending packet...");
        packet.print();

        uint8_t rxBuffer[MAX_RECEIVED_PACKET_SIZE];
        size_t rxBufferSize = sizeof(rxBuffer);
        int err = sbd_modem.sendReceiveSBDBinary(packet.getFullPacket(), packet.getFullPacketLength(), rxBuffer, rxBufferSize);
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

    bool available() override {
        checkSignalQuality();
        receiveIncomingMessages();
        // checkForNewMessages();
        return !receivedPackets.empty();
    }

    Packet read() override {
        if (receivedPackets.empty()) {
            return NullPacket();
        }
        Packet receivedPacket = receivedPackets.front();
        receivedPackets.pop_front();
        return receivedPacket;
    }

private:
    std::deque<Packet> receivedPackets;
    static const size_t MAX_QUEUE_SIZE = 10;
    const uint16_t MAX_RECEIVED_PACKET_SIZE = 340;

    void handleError(int err) {
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
                SerialUSB.println("Protocol error");
                break;
            case ISBD_CANCELLED:
                SerialUSB.println("Cancelled");
                break;
            case ISBD_NO_MODEM_DETECTED:
                SerialUSB.println("No modem detected");
                break;
            case ISBD_SBDIX_FATAL_ERROR:
                SerialUSB.println("SBDIX fatal error");
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
                SerialUSB.println("Unknown error");
                break;
        }
    }

    void storeReceivedPacket(const uint8_t* data, size_t length) {
        // Print the received data
        SerialUSB.print("IridiumPort::storeReceivedPacket() -> Received data: ");
        for (size_t i = 0; i < length; i++) {
            SerialUSB.print(data[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();

        Packet packet(data, length);
        if (!packet.isValid()) {
            SerialUSB.println("IridiumPort::storeReceivedPacket() -> Invalid packet received");
            return;
        }
        if (receivedPackets.size() >= MAX_QUEUE_SIZE) {
            SerialUSB.println("WARNING: Received packet queue is full. Dropping the oldest packet.");
            receivedPackets.pop_front();
        }
        receivedPackets.push_back(packet);
    }

    void receiveIncomingMessages() {
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

    void checkForNewMessages() {
        SerialUSB.println("IridiumPort::checkForNewMessages() -> Checking for new messages (alert || count > 0)...");
        bool ringAlert = sbd_modem.hasRingAsserted();
        int incomingMessages = sbd_modem.getWaitingMessageCount();
        if (!ringAlert && incomingMessages <= 0) {
            return;
        } 
        
        if (ringAlert) {
            SerialUSB.println("IridiumPort::checkRingAlerts() -> Ring alert detected. Checking for incoming messages.");            
        } 
        
        if (incomingMessages > 0) {
            SerialUSB.println("IridiumPort::checkRingAlerts() -> Incoming messages count > 0. Checking for incoming messages.");
        }
        
        receiveIncomingMessages();
    }

    void checkSignalQuality() {
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
};

#endif // IRIDIUMPORT_H
