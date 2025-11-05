#ifndef ACOUSEA_INFRASTRUCTURE_MKR_ROUTINE_MATRIX_UTILS_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_ROUTINE_MATRIX_UTILS_HPP

#include <cstdint>

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

template <size_t N>
constexpr size_t arr_range(const uint8_t (&a)[N]) {
    return static_cast<size_t>(arr_max(a) - arr_min(a) + 1);
}

template <size_t N>
constexpr size_t arr_count(const uint8_t (&)[N]) {
    return N;
}

// ==========================================================
// Constexpr helpers para punteros + tamaño
// ==========================================================

constexpr uint8_t arr_min(const uint8_t* a, const size_t N) {
    uint8_t m = a[0];
    for (size_t i = 1; i < N; ++i)
        if (a[i] < m) m = a[i];
    return m;
}

constexpr uint8_t arr_max(const uint8_t* a, const size_t N) {
    uint8_t m = a[0];
    for (size_t i = 1; i < N; ++i)
        if (a[i] > m) m = a[i];
    return m;
}


constexpr size_t arr_range(const uint8_t* a, const size_t N) {
    return static_cast<size_t>(arr_max(a, N) - arr_min(a, N) + 1);
}


#endif //ACOUSEA_INFRASTRUCTURE_MKR_ROUTINE_MATRIX_UTILS_HPP