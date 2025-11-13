#ifndef ACOUSEA_INFRASTRUCTURE_MKR_DUMMYROUTINE_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_DUMMYROUTINE_HPP

#include "IRoutine.h"
#include "bindings/nodeDevice.pb.h"
#include "Result.h"

class DummyRoutine : public IRoutine<acousea_CommunicationPacket>
{
public:
    mutable int executeCount = 0;
    Result<acousea_CommunicationPacket> resultToReturn;

    explicit DummyRoutine(const char* name)
        : IRoutine<acousea_CommunicationPacket>(name),
          resultToReturn(RESULT_SUCCESS(acousea_CommunicationPacket, acousea_CommunicationPacket_init_default))
    {
    }

    DummyRoutine(const char* name, Result<acousea_CommunicationPacket> result)
        : IRoutine<acousea_CommunicationPacket>(name), resultToReturn(std::move(result))
    {
    }

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket>& input) override
    {
        executeCount++;
        (void)input;
        return resultToReturn;
    }
};

#endif //ACOUSEA_INFRASTRUCTURE_MKR_DUMMYROUTINE_HPP
