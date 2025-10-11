#define ENV_PROD 1
#define ENV_DEV  2
#define ENV_TEST 3

#include "WatchDogMonitor/WatchDogMonitor.h"

#if ENVIRONMENT == ENV_PROD
#include "environment/production/prod_main.h"
#define ENV_SETUP() prod_setup()
#define ENV_LOOP()  prod_loop()

#elif ENVIRONMENT == ENV_DEV
#include "environment/development/dev_main.h"
#define ENV_SETUP() dev_setup()
#define ENV_LOOP()  dev_loop()

#elif ENVIRONMENT == ENV_TEST
#include "environment/testing/test_main.h"
#define ENV_SETUP() test_setup()
#define ENV_LOOP()  test_loop()

#else
#error "No valid ENVIRONMENT defined. Please define ENVIRONMENT as ENV_PROD, ENV_DEV, or ENV_TEST."
#endif


void setup() {
    ENV_SETUP();
}

void loop() {

    ENV_LOOP();
}


// ===================================== Interrupt Handlers =====================================
#ifdef ARDUINO

#if defined(PLATFORM_HAS_LORA)
void onReceiveWrapper(int packetSize) {

#if ENVIRONMENT == ENV_PROD
prod_onReceiveWrapper (packetSize);
#elif ENVIRONMENT == ENV_DEV
dev_onReceiveWrapper (packetSize);
#elif ENVIRONMENT == ENV_TEST
test_onReceiveWrapper (packetSize);
#endif
}
#endif


void SERCOM0_Handler() {
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_DEV
    dev_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM0_Handler();
#endif
}


void SERCOM1_Handler() {
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_DEV
    dev_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM1_Handler();
#endif
}


// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the IridiumPort "mySerial3" to work)
void SERCOM3_Handler() {
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM3_Handler();
#elif ENVIRONMENT == ENV_DEV
    dev_SERCOM3_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM3_Handler();
#endif
}
#endif
