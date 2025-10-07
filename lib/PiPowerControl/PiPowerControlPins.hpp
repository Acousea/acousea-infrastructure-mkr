#ifndef ACOUSEA_PIPOWERCONTROLPINS_HPP
#define ACOUSEA_PIPOWERCONTROLPINS_HPP
#ifdef ARDUINO

// Notice: don't use pin 6. That is LED_BUILTIN for the MKR1310
#define ROCK_PI_SHUTDOWN_PIN A1  // OUTPUT. This pin is driven high to command a shutdown in the rock pi

#define ROCK_PI_MONITOR_PIN  A2  // INPUT pull-down. This is driven high whenever the rock pi is alive

#define MOSFET_CONTROL_PIN  A3  // OUTPUT Mosfeot is switched on when this pin is high

#endif // ARDUINO

#endif //ACOUSEA_PIPOWERCONTROLPINS_HPP
