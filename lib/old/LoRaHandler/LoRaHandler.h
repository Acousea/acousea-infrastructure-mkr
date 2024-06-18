#ifndef _LORA_HANDLER_H_
#define _LORA_HANDLER_H_

#include <LoRa.h>
#include <SPI.h>
#include <config.h>
#include <SDLogger.h>
#include <LoRa.h>
#include <SPI.h>
#include "SDLogger.h"


typedef struct LoRaConfig {
    long frequency;
    float bandwidth;
    uint8_t spreadingFactor;
    uint8_t syncWord;
    uint8_t codingRate;
    uint8_t txPower;
    uint8_t txPin;
};


class LoRaHandler {
public:
    LoRaHandler(const LoRaConfig& config, SDLogger& logger)
    : _config(config), _logger(logger) {}

    bool init() {
        if (!LoRa.begin(_config.frequency)) {
            #if _DEBUG_THROUGH_SERIAL_
                debugPort.println("LoRa failed!");
            #endif
            return false;
        }

        LoRa.setSignalBandwidth(_config.bandwidth);
        LoRa.setSpreadingFactor(_config.spreadingFactor);
        LoRa.setSyncWord(_config.syncWord);
        LoRa.setCodingRate4(_config.codingRate);
        LoRa.setPreambleLength(8);
        LoRa.setTxPower(_config.txPower, _config.txPin);
        
        return true;
    }

    void sendTxtMsg(char *msg) {
        uint16_t msgCount = 0;
        while(!LoRa.beginPacket()) {            
            delay(10);                            
        }
        size_t msg_size_bytes = strlen(msg);

        LoRa.write((uint8_t)(msgCount >> 8));   
        LoRa.write((uint8_t)(msgCount & 0xFF)); 
        LoRa.write((uint8_t)REMOTE_SETUP);      
        LoRa.write(uint8_t(msg_size_bytes));    
        LoRa.write((uint8_t *)msg, msg_size_bytes);
        LoRa.endPacket();
    }

    int sendBinaryMsg(data_info_t data, uint16_t msgCount) {
        uint8_t buffer[250];
        size_t msg_size_bytes;
        bool res = buildDrifterDataPacket(buffer, sizeof(buffer), data, &msg_size_bytes);

        if (!res) return -1;
        while(!LoRa.beginPacket()) {            
            delay(10);                            
        }
        LoRa.write((uint8_t)(msgCount >> 8));   
        LoRa.write((uint8_t)(msgCount & 0xFF)); 
        LoRa.write((uint8_t)REMOTE_DATA_TYPE);  
        LoRa.write(uint8_t(msg_size_bytes));    
        LoRa.write(buffer, msg_size_bytes);
        LoRa.endPacket();                       

        _logger.logCommsBinPacket(buffer, msg_size_bytes, OUT, LORA_CHANNEL);

        return 0;
    }

    int receive(lora_rx_msg_t *msg) {
        msg->timestamp_epoch = rtc.getEpoch();
        msg->type = LoRa.read();
        msg->payload_sz = LoRa.read();
        uint8_t buffer[msg->payload_sz];

        uint8_t nbytes = 0;
        while (nbytes <= msg->payload_sz) {
            buffer[nbytes++] = LoRa.read();
            #if _DEBUG_THROUGH_SERIAL_
                debugPort.print(" 0x");
                debugPort.print(buffer[ nbytes - 1 ], HEX);
            #endif
        }
        #if _DEBUG_THROUGH_SERIAL_
            debugPort.print(" Bytes: ");
            debugPort.println(msg->payload_sz);
        #endif

        return parseMessage(buffer, msg);
    }

private:
    int parseMessage(uint8_t *buffer, lora_rx_msg_t *msg) {
        int error = 0;

        switch (msg->type) {
            case LOCALIZER_ACK:
                #if _DEBUG_THROUGH_SERIAL_
                    debugPort.print("\nType LOCALIZER_ACK sz: ");
                    debugPort.println(msg->payload_sz );
                #endif

                if (msg->payload_sz != 10) {
                    error = -1;
                    break;
                }

                msg->data.ack.msgNo = (uint16_t(buffer[0]) << 8) | uint16_t(buffer[1]);
                memcpy(&(msg->data.ack.latitude), &buffer[2], sizeof(float));   
                memcpy(&(msg->data.ack.longitude), &buffer[6], sizeof(float)); 

                #if _DEBUG_THROUGH_SERIAL_
                    debugPort.print("MsgID: ");
                    debugPort.println(msg->data.ack.msgNo);
                    debugPort.print(" lat: ");
                    debugPort.print(msg->data.ack.latitude, 6),
                    debugPort.print(" lon: ");
                    debugPort.println(msg->data.ack.longitude, 6);
                #endif

                break;

            // Other cases can be handled here

            default:
                error = -1;
        }

        return error;
    }

    const LoRaConfig& _config;
    SDLogger& _logger;
};

#endif