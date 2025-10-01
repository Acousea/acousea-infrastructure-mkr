#ifndef WAITFOR_HPP
#define WAITFOR_HPP
#include "time/getMillis.hpp"


// Versión ligera sin callback
inline void waitFor(const unsigned long durationMs){
    const unsigned long start = getMillis();
    while (getMillis() - start < durationMs){
        // opcional: yield();
    }
}

// Versión con callback
template <typename Callback>
void waitFor(const unsigned long durationMs, const unsigned long tickMs, Callback onTick){
    const unsigned long start = getMillis();
    unsigned long lastTick = 0;
    while (getMillis() - start < durationMs){
        const unsigned long elapsed = getMillis() - start;
        if (elapsed - lastTick >= tickMs){
            lastTick = elapsed;
            onTick(elapsed);
        }
        // opcional: yield();
    }
}

#endif //WAITFOR_HPP
