#ifndef IROUTINE_H
#define IROUTINE_H


#include "Result.h"
#include "bindings/nodeDevice.pb.h"

/**
 * @brief Interfaz base para todas las rutinas del sistema.
 */
template <typename In>
class IRoutine
{
public:
    const char* routineName;  // Nombre estático, sin asignación dinámica


    explicit constexpr IRoutine(const char* name) noexcept
        : routineName(name)
    {
    }

    virtual ~IRoutine() = default;

    // input == nullptr  -> rutina "interna" (sin input)
    // input != nullptr  -> rutina "externa" (usa input)
    virtual Result<acousea_CommunicationPacket> execute(const std::optional<In>& input) = 0;
};

#endif // IROUTINE_H
