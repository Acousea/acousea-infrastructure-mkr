#ifndef GETMILLIS_HPP
#define GETMILLIS_HPP

#ifdef ARDUINO
  #include <Arduino.h>
  static inline unsigned long getMillis() { return millis(); }
#else
#include <chrono>
static inline unsigned long getMillis() {
      using namespace std::chrono;
      static const steady_clock::time_point t0 = steady_clock::now();
      return static_cast<unsigned long>(
          duration_cast<milliseconds>(steady_clock::now() - t0).count()
      );
  }
#endif

#endif //GETMILLIS_HPP
