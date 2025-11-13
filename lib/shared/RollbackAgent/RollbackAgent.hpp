#ifndef ACOUSEA_INFRASTRUCTURE_MKR_ROLLBACKAGENT_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_ROLLBACKAGENT_HPP


#include "ClassName.h"
#include "ProtoUtils/ProtoUtils.hpp"


/**
 * @brief Agente genérico para ejecutar acciones de commit diferido.
 *        No realiza copias ni asignaciones dinámicas: solo guarda punteros.
 *        Rollback implícito: si no se llama commit(), no se ejecuta nada.
 */
class RollbackAgent final
{
    CLASS_NAME(RollbackAgent)

public:
    using Callback = void (*)(void* context);

    struct Action
    {
        Callback fn;
        void* ctx;
    };

    static constexpr size_t MAX_ACTIONS = ProtoUtils::ACOUSEA_MAX_MODULE_COUNT;

    void registerAction(const Callback fn, void* ctx) noexcept;

    void commit() noexcept;

    void clear() noexcept;

    [[nodiscard]] bool empty() const noexcept;

private:
    Action actions[MAX_ACTIONS] = {};
    size_t count{0};
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_ROLLBACKAGENT_HPP
