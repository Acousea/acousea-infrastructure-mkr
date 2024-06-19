#ifndef SERIAL_COMMUNICATOR_H
#define SERIAL_COMMUNICATOR_H

#include <Arduino.h>
#include "ICommunicator.h"
#include "Packet.h"

class SerialCommunicator : public ICommunicator {
private:
    Uart* serialPort;
    int baudRate;

public:
    SerialCommunicator(Uart* serialPort, int baudRate) : serialPort(serialPort), baudRate(baudRate) {
          // Set the pins to use mySerial3
        pinPeripheral(1, PIO_SERCOM); //Assign RX function to pin 1
        pinPeripheral(0, PIO_SERCOM); //Assign TX function to pin 0
    }

    void init(){
        serialPort->begin(baudRate);
        SerialUSB.println("SerialCommunicator::Serial port initialized");
    }
    void send(const Packet& packet) override {
        serialPort->write(packet.getFullPacket(), packet.getFullPacketLength());
    }

    bool available() override {
        // SerialUSB.println("SerialCommunicator::Checking available bytes");
        return serialPort->available() >= Packet::PACKET_HEADER_LENGTH;
    }

    Packet read() override {
        uint8_t buffer[Packet::MAX_PACKET_LENGTH]; 
        size_t index = 0;

        while (serialPort->available() && index < Packet::MAX_PACKET_LENGTH) {
            buffer[index++] = serialPort->read();
        }

        // SerialUSB.print("Received packet: ");
        // for (size_t i = 0; i < index; i++) {
        //     SerialUSB.print(buffer[i], HEX);
        //     SerialUSB.print(" ");
        // }
        // SerialUSB.println("--------");
        
        return Packet(buffer, index);
    }
};

#endif // SERIAL_COMMUNICATOR_H
