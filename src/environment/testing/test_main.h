#ifndef ACOUSEA_INFRASTRUCTURE_MKR_TEST_MAIN_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_TEST_MAIN_HPP

// #include "../lib/MockLib/include/library.h"
#include "dependencies.hpp"
#include "environment/sharedUtils.hpp"

void test_setup();
void test_loop();

#ifdef PLATFORM_ARDUINO
#include <Arduino.h>

#if defined(PLATFORM_HAS_LORA)
void test_onReceiveWrapper(int packetSize);
#endif

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void test_SERCOM3_Handler();
void test_SERCOM1_Handler();
void test_SERCOM0_Handler();
#endif

#endif // ACOUSEA_INFRASTRUCTURE_MKR_TEST_MAIN_HPP
