// #if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
// #include "dependencies.h"
// #include <windows.h>

// #include <chrono>
// #include <thread>
// #include <cstdio>


// void setup();
// void loop();

// static inline void delay_ms(unsigned ms) {
//     std::this_thread::sleep_for(std::chrono::milliseconds(ms));
// }

// // int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
// int main() {
//     setvbuf(stdout, nullptr, _IONBF, 0);
//     setvbuf(stderr, nullptr, _IONBF, 0);
//     std::printf("[native] WinMain: starting...\n");

//     setup();
//     std::printf("[native] setup() done. Entering loop...\n");

//     for (;;) {
//         loop();
//         delay_ms(1); // evita 100% CPU
//     }
// }
// #endif
