#ifndef IRIDIUMPORT_H
#define IRIDIUMPORT_H

#include <Arduino.h>
#include "wiring_private.h"
#include "Ports/IPort.h"
#include "IridiumSBD.h"
#include <deque>

// Define the necessary hardware connections for the Iridium modem
#define SBD_SLEEP_PIN 2 // OUTPUT, pull to GND to switch off
#define SBD_RING_PIN 3 // INPUT, driven LOW when new messages are available
#define SBD_MODEM_BAUDS 19200
#define IridiumSerial mySerial3


// Declare the global variables as extern
extern Uart mySerial3;
extern IridiumSBD sbd_modem;

class IridiumPort : public IPort {

public:
    IridiumPort();

public:
    void init() override;

    void send(const Packet &packet) override;

    bool available() override;

    Result<Packet> read() override;

private:
    std::deque<Packet> receivedPackets;
    static const size_t MAX_QUEUE_SIZE = 10;
    const uint16_t MAX_RECEIVED_PACKET_SIZE = 340;

    void handleError(int err);

    void storeReceivedPacket(const uint8_t *data, size_t length);

    void receiveIncomingMessages();

    void checkRingAlertsAndWaitingMsgCount();

    void checkSignalQuality();
};


#endif // IRIDIUMPORT_H
