#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H


#include <map>
#include "../Routines/IRoutine.h"
#include "Routing/ErrorCode/ErrorCode.h"
#include "Display/IDisplay.h"
#include "Packets/ErrorPacket.h"

class PacketProcessor {
private:
    std::map<OperationCode::Code, IRoutine<Packet> *> routines;

public:
    CLASS_NAME(PacketProcessor)

    PacketProcessor(const std::map<OperationCode::Code, IRoutine<Packet> *> &routines)
            : routines(routines) {}

    Packet process(Packet &packet) {
        SerialUSB.print("Packets Processor: ");
        SerialUSB.println(packet.encode().c_str());
        OperationCode::Code opCode = packet.getOpCodeEnum();
        if (!(routines.find(opCode) != routines.end())) {
            SerialUSB.print(getClassNameCString() + String("Exception: Invalid OpCode"));
            packet.swapSenderReceiverAddresses();
            return ErrorPacket::invalidOpcode(packet.getRoutingChunk());
        }

        Result<Packet> result = routines[opCode]->execute(packet);

        if (result.isError()) {
//          display->print(this->getClassNameString() + "Exception: " + result.getError());
            SerialUSB.print(getClassNameCString() + String("Exception: ") + result.getError().c_str());
            packet.swapSenderReceiverAddresses();
            return ErrorPacket::invalidPayload(packet.getRoutingChunk());
        }

        return result.getValue();

    }
};

#endif