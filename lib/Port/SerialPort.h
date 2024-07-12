#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <Arduino.h>
#include "IPort.h"
#include "Packet.h"

class SerialPort : public IPort {
private:
    Uart* serialPort;
    int baudRate;

public:
    SerialPort(Uart* serialPort, int baudRate) : serialPort(serialPort), baudRate(baudRate) {}

    void init() override{
        serialPort->begin(baudRate);
        SerialUSB.println("SerialPort::Serial port initialized");
    }
    void send(const Packet& packet) override {
        SerialUSB.print("SerialPort::Sending packet: ");
        for (unsigned int i = 0; i < packet.getFullPacketLength(); i++) {
            SerialUSB.print(packet.getFullPacket()[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();
        serialPort->write(packet.getFullPacket(), packet.getFullPacketLength());
    }

    bool available() override {       
        return serialPort->available() >= Packet::PACKET_HEADER_LENGTH;
    }

    Packet read() override {
        uint8_t buffer[Packet::MAX_PACKET_LENGTH]; 
        size_t index = 0;

        while (serialPort->available() && index < Packet::MAX_PACKET_LENGTH) {
            buffer[index++] = serialPort->read();
        }       
        
        return Packet(buffer, index);
    }
};

#endif // SERIAL_PORT_H
