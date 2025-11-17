#ifndef IROUTINE_H
#define IROUTINE_H


#include "Result.h"

/**
 * @brief Interfaz base para todas las rutinas del sistema.
 */
template <typename Type>
class IRoutine
{
public:
    const char* routineName; // Nombre estático, sin asignación dinámica


    explicit constexpr IRoutine(const char* name) noexcept
        : routineName(name)
    {
    }

    virtual ~IRoutine() = default;

    // input == nullptr  -> rutina "interna" (sin input)
    // input != nullptr  -> rutina "externa" (usa input)
    virtual Result<Type*> execute(Type* input) = 0; // CONST POINTER. NOT CONST DATA

    virtual void reset() = 0; // Reset the internal state of the routine
};

#endif // IROUTINE_H
