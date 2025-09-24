#define ENV_PROD 1
#define ENV_DEV  2
#define ENV_TEST 3

#if ENVIRONMENT == ENV_PROD
#include "environment/production/prod_main.h"
void setup(){ return prod_setup(); }
void loop(){ return prod_loop(); }

#elif ENVIRONMENT == ENV_DEV
#include "environment/development/dev_main.h"
void setup() { return dev_setup(); }
void loop() { return dev_loop(); }


#elif ENVIRONMENT == ENV_TEST
#include "environment/testing/test_main.h"
void setup() { return test_setup(); }
void loop() { return test_loop(); }

#else
#error "No valid ENVIRONMENT defined. Please define ENVIRONMENT as ENV_PROD, ENV_DEV, or ENV_TEST."
#endif


#ifdef ARDUINO

#if defined(PLATFORM_HAS_LORA)
void onReceiveWrapper(int packetSize)
{
#if ENVIRONMENT == ENV_PROD
    prod_onReceiveWrapper(packetSize);
#elif ENVIRONMENT == ENV_DEV
    dev_onReceiveWrapper(packetSize);
#elif ENVIRONMENT == ENV_TEST
    test_onReceiveWrapper(packetSize);
#endif
}
#endif


void SERCOM0_Handler(){
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_DEV
    dev_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM0_Handler();
#endif
}


void SERCOM1_Handler(){
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_DEV
    dev_SERCOM0_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM1_Handler();
#endif
}


// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the IridiumPort "mySerial3" to work)
void SERCOM3_Handler(){
#if ENVIRONMENT == ENV_PROD
    prod_SERCOM3_Handler();
#elif ENVIRONMENT == ENV_DEV
    dev_SERCOM3_Handler();
#elif ENVIRONMENT == ENV_TEST
    test_SERCOM3_Handler();
#endif
}
#endif
