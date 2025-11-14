#ifndef CHANGE_MODE_ROUTINE_H
#define CHANGE_MODE_ROUTINE_H

#include "IRoutine.h"
#include "ModuleManager/ModuleManager.hpp"


class SetNodeConfigurationRoutine final : public IRoutine<acousea_CommunicationPacket>
{
private:
    ModuleManager& moduleManager;

public:
    CLASS_NAME(SetNodeConfigurationRoutine)

    explicit SetNodeConfigurationRoutine(ModuleManager& moduleManager);

    Result<acousea_CommunicationPacket*> execute(acousea_CommunicationPacket* inPacketPtr) override;
};


#endif // CHANGE_MODE_ROUTINE_H
