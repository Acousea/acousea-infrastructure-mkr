#ifndef ACOUSEA_MKR1310_NODES_JSONSERIALIZABLE_H
#define ACOUSEA_MKR1310_NODES_JSONSERIALIZABLE_H

#include <ArduinoJson.h>

class JSONSerializable {
public:
    virtual ~JSONSerializable() = default;


    virtual JsonDocument toJson() const = 0;
};

#endif // ACOUSEA_MKR1310_NODES_JSONSERIALIZABLE_H
// Compare this snippet from lib/Packets/Payloads/BasicStatusReportPayload.h: