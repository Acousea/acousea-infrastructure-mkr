#ifndef WAITFOR_HPP
#define WAITFOR_HPP
#include "time/getMillis.hpp"

#ifdef PLATFORM_ARDUINO
#include <Arduino.h>
#include <Adafruit_SleepyDog.h>
#endif

template <typename Condition, typename Callback>
unsigned long waitForOrUntil(const unsigned long durationMs, Condition stopIfTrue,
                    const unsigned long tickMs, Callback onTickCallback)
{
    const unsigned long start = getMillis();
    unsigned long lastTick = 0, elapsed = 0;
#ifdef PLATFORM_ARDUINO
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
#ifdef PLATFORM_ARDUINO
        // Feed the watchdog at a safe interval to avoid excessive resets
        if (elapsed - lastWatchdogReset >= WATCHDOG_FEED_INTERVAL_MS)
        {
            Watchdog.reset();
            lastWatchdogReset = elapsed;
        }
#endif
        // yield();
        delay(5); // Small delay to prevent busy-waiting
    }
    return elapsed;
}


template <typename Condition>
unsigned long waitForOrUntil(const unsigned long durationMs, Condition stopIfTrue)
{
    return waitForOrUntil(durationMs, stopIfTrue, 0, [](unsigned long /*elapsed*/){});
}

// Versión ligera sin callback
inline unsigned long waitFor(const unsigned long durationMs)
{
    return waitForOrUntil(durationMs, [] { return false; },
                   0, [](unsigned long/*elapsed*/){}
    );
}

// Versión con callback
template <typename Callback>
unsigned long waitFor(const unsigned long durationMs, const unsigned long tickMs, Callback onTickCallback)
{
    return waitForOrUntil(durationMs, [] { return false; }, tickMs, onTickCallback);
}


#endif //WAITFOR_HPP
