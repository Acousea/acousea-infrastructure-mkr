#ifndef GETMILLIS_HPP
#define GETMILLIS_HPP

#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <chrono>
#endif

unsigned long getMillis();

#endif // GETMILLIS_HPP
