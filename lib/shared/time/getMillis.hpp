#ifndef GETMILLIS_HPP
#define GETMILLIS_HPP

#ifdef PLATFORM_ARDUINO
  #include <Arduino.h>
#else
  #include <chrono>
#endif

unsigned long getMillis();

#endif // GETMILLIS_HPP
