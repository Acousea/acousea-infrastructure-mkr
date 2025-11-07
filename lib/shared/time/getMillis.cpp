#include "getMillis.hpp"

#ifdef PLATFORM_ARDUINO

unsigned long getMillis() {
    return millis();
}

#else

unsigned long getMillis() {
    using namespace std::chrono;
    static const steady_clock::time_point t0 = steady_clock::now();
    return static_cast<unsigned long>(
        duration_cast<milliseconds>(steady_clock::now() - t0).count()
    );
}

#endif
