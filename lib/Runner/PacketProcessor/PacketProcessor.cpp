#include "PacketProcessor.h"

PacketProcessor::PacketProcessor(const std::map<OperationCode::Code, IRoutine<Packet> *> &routines)
        : routines(routines) {}

Packet PacketProcessor::process(const Packet &packet) {
    SerialUSB.print("Packets Processor: ");
    SerialUSB.println(packet.encode().c_str());
    OperationCode::Code opCode = packet.getOpCodeEnum();
    if (!(routines.find(opCode) != routines.end())) {
        SerialUSB.print(getClassNameCString() + String("Exception: Invalid OpCode"));
        return ErrorPacket::invalidOpcode(packet.getRoutingChunk());
    }

    Result<Packet> result = routines[opCode]->execute(packet);

    if (result.isError()) {
        SerialUSB.print(getClassNameCString() + String("Exception: ") + result.getError().c_str());
        return ErrorPacket::invalidPayload(packet.getRoutingChunk());
    }

    return result.getValue();

}
