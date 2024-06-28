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
    SerialPort(Uart* serialPort, int baudRate) : serialPort(serialPort), baudRate(baudRate) {
        // Set the pins to use mySerial3
        pinPeripheral(1, PIO_SERCOM); //Assign RX function to pin 1
        pinPeripheral(0, PIO_SERCOM); //Assign TX function to pin 0
    }

    void init() override{
        serialPort->begin(baudRate);
        SerialUSB.println("SerialPort::Serial port initialized");
    }
    void send(const Packet& packet) override {
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
