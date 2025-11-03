#ifndef ACOUSEA_INFRASTRUCTURE_MKR_ROUTINEMATRIX_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_ROUTINEMATRIX_HPP

#include <stdint.h>
#include "IRoutine.h"
#include "bindings/nodeDevice.pb.h"

// ==========================================================
// Constexpr helpers para min / max en compilación
// ==========================================================
template <size_t N>
constexpr uint8_t arr_min(const uint8_t (&a)[N]) {
    uint8_t m = a[0];
    for (size_t i = 1; i < N; ++i)
        if (a[i] < m) m = a[i];
    return m;
}

template <size_t N>
constexpr uint8_t arr_max(const uint8_t (&a)[N]) {
    uint8_t m = a[0];
    for (size_t i = 1; i < N; ++i)
        if (a[i] > m) m = a[i];
    return m;
}

/**
 * RoutineGroup:
 *  - El array interno tiene tamaño N (deducido del initList en compilación).
 *  - minTag se pasa en tiempo de ejecución (o constante), y se usa para indexar: idx = tag - minTag.
 *  - Si idx >= N, get() devuelve nullptr y set() no hace nada.
 *  - Sin STL. Arrays C puros. Compatible con constexpr cuando el initList lo es.
 */
template <typename RoutineType, size_t N>
class RoutineGroup {
public:

    constexpr RoutineGroup(const uint8_t (&initTags)[N], const RoutineType* const (&initList)[N]) noexcept
    : minTag_(arr_min(initTags))
    {
        for (size_t i = 0; i < N; ++i)
        {
            routines_[i] = const_cast<RoutineType*>(initList[i]);
        }
    }

    // Set dinámico (por si quieres sobreescribir alguna entrada en runtime)
    void set(uint8_t tag, RoutineType* routine) noexcept {
        const size_t idx = static_cast<size_t>(tag - minTag_);
        if (idx < N) routines_[idx] = routine;
    }

    // Get seguro (nullptr si el tag se sale del rango cubierto por el array)
    constexpr RoutineType* get(uint8_t tag) const noexcept {
        const size_t idx = static_cast<size_t>(tag - minTag_);
        return (idx < N) ? routines_[idx] : nullptr;
    }

    // Helpers
    static constexpr size_t capacity() noexcept { return N; }
    constexpr uint8_t minTag() const noexcept { return minTag_; }
    constexpr uint8_t maxTag() const noexcept { return static_cast<uint8_t>(minTag_ + N - 1); }

private:
    uint8_t     minTag_;
    RoutineType* routines_[N]; // tamaño decidido en compilación por N
};

// ==========================================================
// RoutineMatrix: fija y constexpr
// ==========================================================
struct RoutineMatrix {
    using Routine = IRoutine<acousea_CommunicationPacket>;

    // Tags y arrays de punteros fijos (se definen externamente)
    static constexpr uint8_t CMD_TAGS[]    = {1, 2};
    static constexpr uint8_t RESP_TAGS[]   = {1, 2};
    static constexpr uint8_t REPORT_TAGS[] = {1};

    extern Routine* const CMD_ROUTINES[2];
    extern Routine* const RESP_ROUTINES[2];
    extern Routine* const REPORT_ROUTINES[1];

    static constexpr RoutineGroup<Routine, 2> command{CMD_TAGS, CMD_ROUTINES};
    static constexpr RoutineGroup<Routine, 2> response{RESP_TAGS, RESP_ROUTINES};
    static constexpr RoutineGroup<Routine, 1> report{REPORT_TAGS, REPORT_ROUTINES};

    enum class Type : uint8_t { COMMAND, RESPONSE, REPORT };

    constexpr Routine* get(Type type, uint8_t tag) const {
        switch (type) {
            case Type::COMMAND:  return command.get(tag);
            case Type::RESPONSE: return response.get(tag);
            case Type::REPORT:   return report.get(tag);
            default:             return nullptr;
        }
    }
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_ROUTINEMATRIX_HPP