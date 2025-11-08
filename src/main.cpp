#include "dependencies.hpp"
#include "environment/sharedUtils.hpp"

#if ENVIRONMENT == ENV_PROD
#include "environment/production/prod_main.h"
#define ENV_SETUP() prod_setup()
#define ENV_LOOP()  prod_loop()

#elif ENVIRONMENT == ENV_TEST
#include "environment/testing/test_main.h"
#define ENV_SETUP() test_setup()
#define ENV_LOOP()  test_loop()

#else
#error "No valid ENVIRONMENT defined. Please define ENVIRONMENT as ENV_PROD, or ENV_TEST."
#endif


void setup() {
    ENV_SETUP();
}

void loop() {
    ENV_LOOP();
}


#ifdef PLATFORM_NATIVE
#include <cstdio>
#include <chrono>
#include <thread>

static inline void delay_ms(unsigned ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


#if defined(_WIN32) || defined(__linux__) || defined(__unix__) || defined(__APPLE__)

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    std::printf("[native] main: starting...\n");

    setup();
    std::printf("[native] setup() done. Entering loop...\n");

    for (;;) {
        loop();
        delay_ms(1); // evita 100% CPU
    }
}


#endif // _WIN32 || __linux__ || __unix__ || __APPLE__


#endif // PLATFORM_NATIVE


// ===================================== Interrupt Handlers =====================================
#ifdef PLATFORM_ARDUINO

#if defined(PLATFORM_HAS_LORA)
void onReceiveWrapper(int packetSize) {

#if ENVIRONMENT == ENV_PROD
prod_onReceiveWrapper (packetSize);
#elif ENVIRONMENT == ENV_TEST
test_onReceiveWrapper (packetSize);
#endif
}
#endif


void SERCOM0_Handler() {
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM0_Handler();
#endif
}


void SERCOM1_Handler() {
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM1_Handler();
#endif
}


// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the IridiumPort "mySerial3" to work)
void SERCOM3_Handler() {
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM3_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM3_Handler();
#endif
}
#endif
