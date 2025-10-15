#ifndef IROUTINE_H
#define IROUTINE_H


#include <string>
#include "Result.h"
#include "bindings/nodeDevice.pb.h"

template <typename In>
class IRoutine
{
public:
    std::string routineName;

    explicit IRoutine(std::string name) : routineName(std::move(name))
    {
    }

    virtual ~IRoutine() = default;

    // input == nullptr  -> rutina "interna" (sin input)
    // input != nullptr  -> rutina "externa" (usa input)
    virtual Result<acousea_CommunicationPacket> execute(const std::optional<In>& input) = 0;
};

#endif // IROUTINE_H
