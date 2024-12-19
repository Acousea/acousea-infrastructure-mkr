#ifndef CHANGE_MODE_ROUTINE_H
#define CHANGE_MODE_ROUTINE_H

#include "IRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"


class SetNodeConfigurationRoutine : public IRoutine<Packet> {
private:
    NodeConfigurationRepository &nodeConfigurationRepository;

public:
    CLASS_NAME(SetNodeConfigurationRoutine)

    SetNodeConfigurationRoutine(NodeConfigurationRepository &nodeConfigurationRepository);

    Result<Packet> execute(const Packet &packet) override;

private:
    static void setOperationModes(NodeConfiguration &nodeConfig, const SerializableModule &item);
    static void setReportingPeriods(NodeConfiguration &nodeConfig, const SerializableModule &item);

};

#endif // CHANGE_MODE_ROUTINE_H
