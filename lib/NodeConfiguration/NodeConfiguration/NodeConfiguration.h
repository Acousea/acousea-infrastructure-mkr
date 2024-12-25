#ifndef ACOUSEA_MKR1310_NODES_NODECONFIGURATION_H
#define ACOUSEA_MKR1310_NODES_NODECONFIGURATION_H

#include <map>
#include <optional>
#include <utility>
#include "ArduinoJson.h"
#include "Payload/Modules/extModules/reporting/ReportingModule.h"
#include "Payload/Modules/extModules/operationModes/OperationModesGraphModule.h"
#include "Payload/Modules/extModules/reporting/iridium/IridiumReportingPeriodModule.h"
#include "Payload/Modules/extModules/reporting/loRa/LoRaReportingPeriodModule.h"
#include "Result/Result.h"
#include "Routing/Address/Address.h"



class NodeConfiguration {
private:
    Address localAddress;
    std::optional<OperationModesGraphModule> operationGraphModule;
    std::optional<LoRaReportingModule> loraModule;
    std::optional<IridiumReportingModule> iridiumModule;

public:
    NodeConfiguration(const Address &localAddress, const std::optional<OperationModesGraphModule> &operationGraphModule,
                      const std::optional<LoRaReportingModule> &loraModule,
                      const std::optional<IridiumReportingModule> &iridiumModule);

    void print() const;

    [[nodiscard]] const std::optional<LoRaReportingModule> &getLoraModule() const;

    [[nodiscard]] const std::optional<IridiumReportingModule> &getIridiumModule() const;

    [[nodiscard]] const std::optional<OperationModesGraphModule> &getOperationGraphModule() const;

    void setLocalAddress(const Address &localAddress);

    void setOperationGraphModule(const OperationModesGraphModule &module);

    void setLoraModule(const LoRaReportingModule &loraModule);

    void setIridiumModule(const IridiumReportingModule &iridiumModule);

    static NodeConfiguration getDefaultConfiguration();

    [[nodiscard]] const Address &getLocalAddress() const;

    static Result<NodeConfiguration> fromJson(const std::string &json);

    [[nodiscard]] std::string toJson() const;
};

#endif // ACOUSEA_MKR1310_NODES_NODECONFIGURATION_H
