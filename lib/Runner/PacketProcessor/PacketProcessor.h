#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H


#include <map>
#include "IRoutine.h"
#include "Routing/ErrorCode/ErrorCode.h"
#include "Packets/ErrorPacket.h"

class PacketProcessor {
private:
    std::map<OperationCode::Code, IRoutine<Packet> *> routines;

public:
    CLASS_NAME(PacketProcessor)

    PacketProcessor(const std::map<OperationCode::Code, IRoutine<Packet> *> &routines);

    Packet process(const Packet &packet);
};

#endif