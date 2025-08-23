// src/entry_native.cpp
#if defined(PLATFORM_NATIVE) && !defined(ARDUINO)

#include "dependencies.h"
#include <cstdio>
#include <chrono>
#include <thread>

static inline void delay_ms(unsigned ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void setup();
void loop();

#if defined(_WIN32)

// -------- Windows ----------
int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    std::printf("[native][win] main: starting...\n");

    setup();
    std::printf("[native][win] setup() done. Entering loop...\n");

    for (;;) {
        loop();
        delay_ms(1); // evita 100% CPU
    }
}

#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)

// -------- POSIX (Linux/macOS/BSD) ----------
int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    std::puts("[native][posix] main: starting...");

    setup();
    std::puts("[native][posix] setup() done. Entering loop...");

    for (;;) {
        loop();
        delay_ms(1); // evita 100% CPU
    }
}

#else
#  error "PLATFORM_NATIVE definido pero plataforma no reconocida"
#endif

#endif // PLATFORM_NATIVE && !ARDUINO
