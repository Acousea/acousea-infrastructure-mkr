#ifndef SBD_MODEM_H
#define SBD_MODEM_H

#include <Arduino.h>
#include <IridiumSBD.h>
#include "SDLogger.h"
#include "RTCManager.h"

struct SBDModemConfig {
    uint32_t baudRate;
    uint8_t sleepPin;
    uint8_t ringPin;
};

class SBDModem {
public:
    SBDModem(const SBDModemConfig& config, IridiumSBD& modem, HardwareSerial& serial, SDLogger& logger, RTCManager& rtcManager)
    : _config(config), _modem(modem), _serial(serial), _logger(logger), _rtcManager(rtcManager), _isOn(false) {}

    bool init(bool atSetup, int& signalQuality) {
        char debugStr[100];
        snprintf(debugStr, sizeof(debugStr), "Initializing IRIDIUM SBD modem ...\n");
        _logger.logDebugString(debugStr);

        _serial.begin(_config.baudRate);
        int err = _modem.begin();
        if (err != ISBD_SUCCESS) {
            handleError(err, "SBD modem begin failed");
            return false;
        }
        _isOn = true;

        _modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
        _modem.enableRingAlerts(true);

        if (atSetup) {
            if (!getFirmwareVersion() || !getIMEI()) {
                return false;
            }
        }

        #if _SBD_INDOORS_ == FALSE
            signalQuality = getSignalQuality();
            if (signalQuality < 0) {
                return false;
            }
        #else
            signalQuality = -5;
        #endif

        snprintf(debugStr, sizeof(debugStr), "Signal quality (0-5): %d\n", signalQuality);
        _logger.logDebugString(debugStr);
        snprintf(debugStr, sizeof(debugStr), "IRIDIUM SBD modem initialized\n");
        _logger.logDebugString(debugStr);

        return true;
    }

    int sendBinaryMsg(data_info_t data) {
        int err = ISBD_SUCCESS;
        size_t msgSizeBytes;
        uint8_t buffer[340];

        bool res = buildDrifterDataPacket(buffer, sizeof(buffer), data, &msgSizeBytes);
        if (!res) return -1;

        #if _SBD_INDOORS_ == false
            char debugStr[100];
            bool messageSent = false;

            if (!messageSent || _modem.getWaitingMessageCount() > 0) {
                size_t bufferSize = sizeof(buffer);

                if (!messageSent) {
                    _logger.logCommsBinPacket(buffer, msgSizeBytes, OUT, SBD_CHANNEL);
                    err = _modem.sendReceiveSBDBinary(buffer, msgSizeBytes, buffer, bufferSize);
                    if (err != ISBD_SUCCESS) {
                        snprintf(debugStr, sizeof(debugStr), "ERROR: Sending drifter data packet through SBD modem failed with err: %d\n", err);
                    } else {
                        messageSent = true;
                        snprintf(debugStr, sizeof(debugStr), "Drifter data packet sent through SBD modem\n");
                    }
                    _logger.logDebugString(debugStr);
                } else {
                    err = _modem.sendReceiveSBDBinary(NULL, 0, buffer, bufferSize);
                    if (err != ISBD_SUCCESS) {
                        snprintf(debugStr, sizeof(debugStr), "ERROR: Receiving a data packet through SBD modem failed with err: %d\n", err);
                    } else {
                        snprintf(debugStr, sizeof(debugStr), "Drifter data packet (%u bytes) received through SBD modem\n", bufferSize);
                    }
                    _logger.logDebugString(debugStr);
                }

                if (bufferSize > 0) {
                    parseIncomingPacket(buffer, bufferSize);
                    snprintf(debugStr, sizeof(debugStr), "SBD: MT pending messages: %d\n", _modem.getWaitingMessageCount());
                    _logger.logDebugString(debugStr);
                }

                err = _modem.clearBuffers(ISBD_CLEAR_MO);
                if (err != ISBD_SUCCESS) {
                    snprintf(debugStr, sizeof(debugStr), "SBD: Clearing MO buffer failed (err: %d)", err);
                    _logger.logDebugString(debugStr);
                }
            }
        #else
            _logger.logCommsBinPacket(buffer, msgSizeBytes, OUT, SBD_CHANNEL);
        #endif

        return err;
    }

    void shutdown() {
        _modem.sleep();
        _isOn = false;
        _serial.end();

        char debugStr[50];
        snprintf(debugStr, sizeof(debugStr), "SBD modem is OFF\n");
        _logger.logDebugString(debugStr);
    }

private:
    const SBDModemConfig& _config;
    IridiumSBD& _modem;
    HardwareSerial& _serial;
    SDLogger& _logger;
    RTCManager& _rtcManager;
    bool _isOn;

    void handleError(int err, const char* context) {
        char debugStr[100];
        if (err == ISBD_NO_MODEM_DETECTED) {
            snprintf(debugStr, sizeof(debugStr), "ERROR: No modem detected: check wiring\n");
        } else {
            snprintf(debugStr, sizeof(debugStr), "ERROR: %s with error %d\n", context, err);
        }
        _logger.logDebugString(debugStr);
    }

    bool getFirmwareVersion() {
        char version[12];
        int err = _modem.getFirmwareVersion(version, sizeof(version));
        if (err != ISBD_SUCCESS) {
            handleError(err, "getFirmwareVersion() failed");
            return false;
        }
        char debugStr[100];
        snprintf(debugStr, sizeof(debugStr), "Firmware version: %s\n", version);
        _logger.logDebugString(debugStr);
        return true;
    }

    bool getIMEI() {
        char IMEI[16];
        int err = _modem.getIMEI(IMEI, sizeof(IMEI));
        if (err != ISBD_SUCCESS) {
            handleError(err, "getIMEI() failed");
            return false;
        }
        char debugStr[100];
        snprintf(debugStr, sizeof(debugStr), "IMEI: %s\n", IMEI);
        _logger.logDebugString(debugStr);
        return true;
    }

    int getSignalQuality() {
        int signalQuality = -1;
        int err = _modem.getSignalQuality(signalQuality);
        if (err != ISBD_SUCCESS) {
            handleError(err, "getSignalQuality() failed");
            return -1;
        }
        return signalQuality;
    }
};

#endif