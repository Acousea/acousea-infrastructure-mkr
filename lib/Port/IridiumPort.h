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
        IridiumSerial.begin(SBD_MODEM_BAUDS);
        int err = sbd_modem.begin();
        if (err != ISBD_SUCCESS) {
            handleError(err);
        }
        sbd_modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
        sbd_modem.enableRingAlerts(true);
    }

    void send(const Packet& packet) override {
        SerialUSB.print("IridiumPort::send(): Sending packet: ");
        for (size_t i = 0; i < packet.getFullPacketLength(); i++) {
            SerialUSB.print(packet.getFullPacket()[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();
        uint8_t rxBuffer[MAX_RECEIVED_PACKET_SIZE];
        size_t rxBufferSize = sizeof(rxBuffer);
        int err = sbd_modem.sendReceiveSBDBinary(packet.getFullPacket(), packet.getFullPacketLength(), rxBuffer, rxBufferSize);
        if (err != ISBD_SUCCESS) {
            handleError(err);
        } else if (rxBufferSize > 0) {
            storeReceivedPacket(rxBuffer, rxBufferSize);
        }
    }

    bool available() override {
        return !receivedPackets.empty();
    }

    // FIXME: Must implement logic to read the received packets from the modem
    /**
     *  uint8_t rxBuffer[340];
        size_t rxBufferSize = sizeof(rxBuffer);
        int err = sbd_modem.sendReceiveSBDBinary(NULL, 0, rxBuffer, rxBufferSize);
        if (err != ISBD_SUCCESS) {
            handleError(err);
            return NullPacket();
        }
        return Packet(rxBuffer, rxBufferSize);
     */
    Packet read() override {
        if (!available()) {
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
};

#endif // IRIDIUMPORT_H
