#ifndef ENSURE_HPP
#define ENSURE_HPP

#include <cstdio>
#include <cstdlib> // std::abort

#ifdef ARDUINO
#include <Arduino.h>
// Asumo que tienes ConsoleSerial ya inicializado
#endif

template <typename T>
inline void ENSURE(T* ptr, const char* name){
    if (ptr) return;

#ifdef ARDUINO
    // Parpadeo infinito en el LED
    if (SerialUSB){
        SerialUSB.print("[arduino] ");
        SerialUSB.print(name);
        SerialUSB.println(" is NULL");
    }
    pinMode(LED_BUILTIN, OUTPUT);
    while (true){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
    }
#else
    std::fprintf(stderr, "[native] %s is NULL\n", name);
    std::abort();  // mata el programa inmediatamente
#endif
}

template <typename Func>
void executeEvery(const unsigned long interval, Func&& callback){
    static unsigned long lastTime = 0;
    unsigned long now = getMillis();
    if (lastTime == 0 || now - lastTime >= interval){
        callback();
        lastTime = now;
    }
}


#endif //ENSURE_HPP
