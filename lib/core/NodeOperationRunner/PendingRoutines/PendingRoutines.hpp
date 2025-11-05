#ifndef ACOUSEA_INFRASTRUCTURE_MKR_PENDINGROUTINES_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_PENDINGROUTINES_HPP

#include <optional>
#include <IRoutine.h>
#include <Ports/IPort.h>

template <size_t MAX_ROUTINES>
struct PendingRoutines
{
    using Packet = acousea_CommunicationPacket;
    using Routine = IRoutine<Packet>;

    struct Entry
    {
        Routine* routine = nullptr;
        std::optional<Packet> input{};
        uint8_t attempts = 0;
        IPort::PortType portResponseTo{IPort::PortType::None};
    };

public:
    PendingRoutines() : head(0), tail(0), count(0)
    {
    }

    void add(const Entry& e)
    {
        buffer[tail] = e;
        tail = (tail + 1) % MAX_ROUTINES;

        if (count < MAX_ROUTINES)
        {
            ++count;
        }
        else
        {
            // sobrescribir el más antiguo
            head = (head + 1) % MAX_ROUTINES;
        }
    }

    std::optional<Entry> next()
    {
        if (count == 0)
            return std::nullopt;

        Entry e = buffer[head];
        head = (head + 1) % MAX_ROUTINES;
        --count;
        return e;
    }

    [[nodiscard]] bool empty() const { return count == 0; }
    [[nodiscard]] size_t size() const { return count; }

    static constexpr const uint8_t MAX_ATTEMPTS = 3;

private:
    Entry buffer[MAX_ROUTINES];
    size_t head;
    size_t tail;
    size_t count;
};

#endif // ACOUSEA_INFRASTRUCTURE_MKR_PENDINGROUTINES_HPP
