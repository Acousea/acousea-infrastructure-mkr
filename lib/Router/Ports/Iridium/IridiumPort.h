#ifndef IRIDIUMPORT_H
#define IRIDIUMPORT_H

#ifdef PLATFORM_ARDUINO

#include <Arduino.h>

#include "ClassName.h"
#include "wiring_private.h"
#include "Ports/IPort.h"
#include "IridiumSBD.h"

// Define the necessary hardware connections for the Iridium modem
#define SBD_TX_PIN 0 // OUTPUT, connect to RX of Iridium
#define SBD_RX_PIN 1 // INPUT, connect to TX of Iridium
#define SBD_SLEEP_PIN 2 // OUTPUT, pull to GND to switch off
#define SBD_RING_PIN 3 // INPUT, driven LOW when new messages are available
#define SBD_NET_AVAILABLE_PIN 4 // INPUT, driven HIGH when network is available
#define SBD_MODEM_BAUDS 19200
#define IridiumSerial mySerial3

// Declare the global variables as extern
extern Uart mySerial3;
extern IridiumSBD sbd_modem;

class IridiumPort : public IPort {
    CLASS_NAME(IridiumPort)

public:
    IridiumPort();

public:
    void init() override;

    bool send(const std::vector<uint8_t>& data) override;

    bool available() override;

    std::vector<std::vector<uint8_t>> read() override;

private:

    static void handleError(int err);

    void storeReceivedPacket(const uint8_t *data, size_t length);

    void receiveIncomingMessages();

    void checkRingAlertsAndWaitingMsgCount();

    void checkSignalQuality();
};


#endif // ARDUINO

#endif // IRIDIUMPORT_H
