#ifndef ACOUSEA_MKR1310_NODES_LORAREPORTINGMODULE_H
#define ACOUSEA_MKR1310_NODES_LORAREPORTINGMODULE_H

#include "Payload/Modules/extModules/reporting/ReportingModule.h"
#include "Payload/Modules/JSONSerializable.h"

class LoRaReportingModule : public ReportingModule, public JSONSerializable {
public:

    explicit LoRaReportingModule(const std::map<uint8_t, ReportingConfiguration> &configs);

    static LoRaReportingModule fromJSON(const JsonArrayConst &doc);

    [[nodiscard]] JsonDocument toJson() const override;

};

#endif // ACOUSEA_MKR1310_NODES_LORAREPORTINGMODULE_H
