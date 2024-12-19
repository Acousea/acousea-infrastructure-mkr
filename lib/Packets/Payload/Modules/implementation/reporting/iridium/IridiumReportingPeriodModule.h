#ifndef ACOUSEA_MKR1310_NODES_IRIDIUMREPORTINGMODULE_H
#define ACOUSEA_MKR1310_NODES_IRIDIUMREPORTINGMODULE_H

#include "Payload/Modules/implementation/reporting/ReportingModule.h"
#include "Payload/Modules/JSONSerializable.h"

class IridiumReportingModule : public ReportingModule, public JSONSerializable {
public:

    explicit IridiumReportingModule(const std::map<uint8_t, ReportingConfiguration> &configs);

    static IridiumReportingModule fromJSON(const JsonArrayConst &doc);

    [[nodiscard]] JsonDocument toJson() const override;

};

#endif // ACOUSEA_MKR1310_NODES_IRIDIUMREPORTINGMODULE_H
