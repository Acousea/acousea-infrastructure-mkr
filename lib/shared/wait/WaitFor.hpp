#ifndef WAITFOR_HPP
#define WAITFOR_HPP
#include "time/getMillis.hpp"

#ifdef ARDUINO
#include <Arduino.h>
#include <Adafruit_SleepyDog.h>
#endif

template <typename Condition, typename Callback>
void waitForOrUntil(const unsigned long durationMs, Condition stopIfTrue,
                    const unsigned long tickMs, Callback onTickCallback)
{
    const unsigned long start = getMillis();
    unsigned long lastTick = 0, elapsed = 0;
#ifdef ARDUINO
    unsigned long lastWatchdogReset = 0;
    constexpr unsigned long WATCHDOG_FEED_INTERVAL_MS = 500;
#endif
    while (!stopIfTrue() && elapsed < durationMs)
    {
        elapsed = getMillis() - start;
        if (elapsed - lastTick >= tickMs && tickMs > 0)
        {
            lastTick = elapsed;
            onTickCallback(elapsed);
        }
#ifdef ARDUINO
        // Feed the watchdog at a safe interval to avoid excessive resets
        if (elapsed - lastWatchdogReset >= WATCHDOG_FEED_INTERVAL_MS)
        {
            Watchdog.reset();
            lastWatchdogReset = elapsed;
        }
#endif
        // yield();
        // delay(100);
    }

}


template <typename Condition>
void waitForOrUntil(const unsigned long durationMs, Condition stopIfTrue)
{
    waitForOrUntil(durationMs, stopIfTrue, 0, [](unsigned long /*elapsed*/){});
}

// Versión ligera sin callback
inline void waitFor(const unsigned long durationMs)
{
    waitForOrUntil(durationMs, [] { return false; },
                   0, [](unsigned long/*elapsed*/){}
    );
}

// Versión con callback
template <typename Callback>
void waitFor(const unsigned long durationMs, const unsigned long tickMs, Callback onTickCallback)
{
    waitForOrUntil(durationMs, [] { return false; }, tickMs, onTickCallback);
}


#endif //WAITFOR_HPP
