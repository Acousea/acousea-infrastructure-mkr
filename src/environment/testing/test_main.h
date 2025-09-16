#include "test_dependencies.h"
// #include "../lib/MockLib/include/library.h"

#define ENSURE(ptr, name) do{ if(!(ptr)){ std::fprintf(stderr,"[native] %s is NULL\n", name); return; } }while(0)

void test_setup();
void test_loop();

#ifdef ARDUINO

void test_onReceiveWrapper(int packetSize);
// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void test_SERCOM3_Handler();

#endif


