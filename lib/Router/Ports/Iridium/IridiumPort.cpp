#ifdef PLATFORM_ARDUINO
#include "IridiumPort.h"

#include <Logger/Logger.h>

#include "SharedMemory/SharedMemory.hpp"

Uart mySerial3(&sercom3, SBD_RX_PIN, SBD_TX_PIN, SERCOM_RX_PAD_1, UART_TX_PAD_0);

// Global instance of the IridiumSBD modem
IridiumSBD sbd_modem(IridiumSerial, SBD_SLEEP_PIN, SBD_RING_PIN);

#define DIAGNOSTICS 1
#if DIAGNOSTICS

void ISBDConsoleCallback(IridiumSBD* device, char c)
{
    Serial.write(c);
}

void ISBDDiagsCallback(IridiumSBD* device, char c)
{
    Serial.write(c);
}

#endif

IridiumPort::IridiumPort(PacketQueue& packetQueue) : IPort(PortType::SBDPort), packetQueue_(packetQueue)
{
}

void IridiumPort::init()
{
    // Set the pins to use mySerial3
    // pinPeripheral(0, PIO_SERCOM); // Assign TX function to pin 0
    // pinPeripheral(1, PIO_SERCOM); // Assign RX function to pin 1

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
    // WARNING: Must check if its really necessary to call pinPeripheral, and if so, if this call should
    // be done before or after sbd_modem.begin(), most likely after, due to the internal Uart.begin() implementation
    IridiumSerial.begin(SBD_MODEM_BAUDS);

    LOG_CLASS_INFO("IridiumPort::init() -> Initializing Iridium modem...");

    if (const int err = sbd_modem.begin(); err != ISBD_SUCCESS)
    {
        logError(err);
    }
    sbd_modem.enableRingAlerts(true); // Documentations says this should go before begin(), but not sure if it works
    LOG_CLASS_INFO("IridiumPort::init() -> Iridium modem initialized");
}

bool IridiumPort::send(const uint8_t* data, const size_t length)
{
    LOG_CLASS_INFO("IridiumPort::send() -> Sending packet... Data: %s, Size: %d bytes",
                   Logger::vectorToHexString(data, length).c_str(), length
    );

    auto* rxBuffer = SharedMemory::tmpBuffer();
    size_t rxBufferSize = SharedMemory::tmpBufferSize();

    const int err = sbd_modem.sendReceiveSBDBinary(data, length, rxBuffer, rxBufferSize);
    if (err != ISBD_SUCCESS)
    {
        logError(err);
        return false;
    }
    // Store received packet if available
    if (rxBufferSize > 0)
    {
        storeReceivedPacket(rxBuffer, rxBufferSize);
    }

    // Check for additional waiting messages if any
    if (sbd_modem.getWaitingMessageCount() > 0)
    {
        _receiveIncomingMessages();
    }
    return true;
}

bool IridiumPort::available()
{
    return !packetQueue_.isEmpty();
}

/// Synchronize the port: check for ring alerts and incoming messages
bool IridiumPort::sync()
{
    checkSignalQuality();
    const bool ringAlert = sbd_modem.hasRingAsserted();
    const int incomingMessages = sbd_modem.getWaitingMessageCount();

    LOG_CLASS_INFO(
        "IridiumPort::checkRingAlerts() -> hasRingAsserted(): %s, getWaitingMessageCount(): %d",
        ringAlert ? "true" : "false",
        incomingMessages
    );

    if (ringAlert || incomingMessages > 0)
    {
        LOG_CLASS_INFO("IridiumPort::checkRingAlerts() -> Checking for incoming messages.");
        _receiveIncomingMessages();
    }
    return true;
}


void IridiumPort::logError(const int err)
{
    char errorMessage[128]; // tamaño ajustable según tus logs
    int written = snprintf(errorMessage, sizeof(errorMessage), "logError(): %d - ", err);

    const char* desc = nullptr;
    switch (err)
    {
    case ISBD_SUCCESS: desc = "Success";
        break;
    case ISBD_ALREADY_AWAKE: desc = "Already awake";
        break;
    case ISBD_SERIAL_FAILURE: desc = "Serial failure";
        break;
    case ISBD_PROTOCOL_ERROR: desc = "Protocol failure";
        break;
    case ISBD_CANCELLED: desc = "Cancelled";
        break;
    case ISBD_NO_MODEM_DETECTED: desc = "No modem detected";
        break;
    case ISBD_SBDIX_FATAL_ERROR: desc = "SBDIX fatal failure";
        break;
    case ISBD_SENDRECEIVE_TIMEOUT: desc = "Send/receive timeout";
        break;
    case ISBD_RX_OVERFLOW: desc = "RX overflow";
        break;
    case ISBD_REENTRANT: desc = "Reentrant";
        break;
    case ISBD_IS_ASLEEP: desc = "Is asleep";
        break;
    case ISBD_NO_SLEEP_PIN: desc = "No sleep pin";
        break;
    case ISBD_NO_NETWORK: desc = "No network";
        break;
    case ISBD_MSG_TOO_LONG: desc = "Message too long";
        break;
    default: desc = "Unknown failure";
        break;
    }

    // Aseguramos no sobrepasar el buffer
    if (written < 0) written = 0;
    snprintf(errorMessage + written, sizeof(errorMessage) - written, "%s", desc);

    LOG_CLASS_ERROR("%s", errorMessage);
}


void IridiumPort::storeReceivedPacket(const uint8_t* data, size_t length)
{
    // Print the received data
    LOG_CLASS_INFO("::storeReceivedPacket() -> Received data: %s",
                   Logger::vectorToHexString(data, length).c_str()
    );

    const bool pushOk = packetQueue_.push(getTypeU8(), data, static_cast<uint16_t>(length));
    if (!pushOk)
    {
        LOG_CLASS_ERROR("::storeReceivedPacket() -> Failed to store received packet in flash queue.");
    }
    LOG_CLASS_INFO("::storeReceivedPacket() -> Stored received packet in flash queue.");
}

void IridiumPort::_receiveIncomingMessages()
{
    LOG_CLASS_INFO("IridiumPort::receiveIncomingMessages() -> Checking for incoming messages...");
    auto* rxBuffer = SharedMemory::tmpBuffer();
    size_t rxBufferSize = SharedMemory::tmpBufferSize();
    do
    {
        if (const int err = sbd_modem.sendReceiveSBDBinary(NULL, 0, rxBuffer, rxBufferSize); err != ISBD_SUCCESS)
        {
            logError(err);
            break;
        }
        if (rxBufferSize <= 0)
        {
            LOG_CLASS_INFO("IridiumPort::receiveIncomingMessages() -> No data read.");
            continue;
        }
        storeReceivedPacket(rxBuffer, rxBufferSize);
        SharedMemory::clearTmpBuffer();
    }
    while (sbd_modem.getWaitingMessageCount() > 0);
}

void IridiumPort::checkSignalQuality()
{
    LOG_CLASS_INFO("IridiumPort::checkSignalQuality() -> Checking signal quality...");
    int signalQuality = -1;
    if (const int err = sbd_modem.getSignalQuality(signalQuality); err != ISBD_SUCCESS)
    {
        logError(err);
        return;
    }
    LOG_CLASS_INFO(
        "IridiumPort::checkSignalQuality() -> Signal quality: %d",
        signalQuality
    );
}

#endif // ARDUINO
