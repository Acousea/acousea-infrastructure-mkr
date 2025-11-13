#ifndef ENSURE_HPP
#define ENSURE_HPP

#include <cstdio>
#include <cstdlib> // std::abort
#include "time/getMillis.hpp"



namespace SharedUtils
{
    template <typename Func>
    void executeEvery(const unsigned long interval, Func&& callback)
    {
        static unsigned long lastTime = 0;
        unsigned long now = getMillis();
        if (lastTime == 0 || now - lastTime >= interval)
        {
            callback();
            lastTime = now;
        }
    }

#if defined(PLATFORM_ARDUINO)
#include <Arduino.h>


    template <typename Func>
    void withLedIndicator(Func&& innerFunc)
    {
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, HIGH); // LED ON mientras se ejecuta

        innerFunc(); // Ejecuta la función interna

        digitalWrite(LED_BUILTIN, LOW); // LED OFF al termina
    }


#elif defined(PLATFORM_NATIVE)

    template <typename Func>
    void withLedIndicator(Func&& innerFunc)
    {
        innerFunc(); // Ejecuta la función interna
    }

#include <iostream>
#include <string>

    struct NativeConsole
    {
        template <typename T>
        void print(const T& value) const
        {
            std::cout << value;
        }

        template <typename T>
        void println(const T& value) const
        {
            std::cout << value << std::endl;
        }

        void println() const
        {
            std::cout << std::endl;
        }
    };

    static const NativeConsole ConsoleSerial;

#else
#error "Platform not supported for ENSURE and withLedIndicator"
#endif
}
#endif //ENSURE_HPP
