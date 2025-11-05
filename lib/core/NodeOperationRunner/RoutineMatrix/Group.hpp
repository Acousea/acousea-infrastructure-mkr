#ifndef ACOUSEA_INFRASTRUCTURE_MKR_GROUP_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_GROUP_HPP

#include <cstdint>
#include "Utils.hpp"

/**
 * @brief Contenedor genérico indexado por tag.
 *        - MIN y MAX se determinan en el constructor a partir de los tags iniciales.
 *        - Usa memoria dinámica para items_ (T**), sin STL.
 *        - No realiza copias, sólo guarda punteros.
 */
template <typename T>
class Group
{
public:
    constexpr Group(const uint8_t* initTags, T* const* initList, const size_t count) noexcept
        : minTag_(arr_min(initTags, count)), maxTag_(arr_max(initTags, count)), capacity_(arr_range(initTags, count))
    {
        // Reserva memoria dinámica para los punteros
        items_ = new T**[capacity_];
        for (size_t i = 0; i < capacity_; ++i)
            items_[i] = nullptr;

        // Rellena según los tags
        for (size_t i = 0; i < count; ++i)
        {
            const uint8_t tag = initTags[i];
            if (initList[i] == nullptr) continue;
            if (tag < minTag_ || tag > maxTag_) continue;

            const auto idx = static_cast<size_t>(tag - minTag_);
            items_[idx] = const_cast<T**>(&initList[i]);
        }
    }

    // Destructor — libera la memoria
    ~Group()
    {
        delete[] items_;
        items_ = nullptr;
    }

    // Evita copia (propietario de memoria)
    Group(const Group&) = delete;
    Group& operator=(const Group&) = delete;

    // Permite movimiento
    constexpr Group(Group&& other) noexcept
        : minTag_(other.minTag_),
          maxTag_(other.maxTag_),
          capacity_(other.capacity_),
          items_(other.items_)
    {
        other.items_ = nullptr;
        other.capacity_ = 0;
    }

    constexpr Group& operator=(Group&& other) noexcept
    {
        if (this != &other)
        {
            delete[] items_;
            minTag_ = other.minTag_;
            maxTag_ = other.maxTag_;
            capacity_ = other.capacity_;
            items_ = other.items_;
            other.items_ = nullptr;
            other.capacity_ = 0;
        }
        return *this;
    }

    [[nodiscard]] constexpr uint8_t minTag() const noexcept { return minTag_; }
    [[nodiscard]] constexpr uint8_t maxTag() const noexcept { return maxTag_; }
    [[nodiscard]] constexpr size_t capacity() const noexcept { return capacity_; }

    [[nodiscard]] constexpr T* get(const uint8_t tag) const noexcept
    {
        if (!items_ || tag < minTag_ || tag > maxTag_) return nullptr;
        T** ptrptr = items_[tag - minTag_];
        return ptrptr ? *ptrptr : nullptr;
    }

    [[nodiscard]] constexpr bool contains(const uint8_t tag) const noexcept
    {
        if (!items_ || tag < minTag_ || tag > maxTag_) return false;
        T** ptrptr = items_[tag - minTag_];
        return ptrptr && *ptrptr;
    }

    void clear() noexcept {
        for (size_t i = 0; i < capacity_; ++i)
            items_[i] = nullptr;
    }


private:
    uint8_t minTag_;
    uint8_t maxTag_;
    size_t capacity_;
    T*** items_ = nullptr; // array dinámico de punteros a punteros
};

#endif // ACOUSEA_INFRASTRUCTURE_MKR_GROUP_HPP
