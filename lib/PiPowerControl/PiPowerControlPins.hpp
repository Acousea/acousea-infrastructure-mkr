#ifndef ACOUSEA_MKR1310_NODES_PIPOWERCONTROLPINS_HPP
#define ACOUSEA_MKR1310_NODES_PIPOWERCONTROLPINS_HPP
#ifdef ARDUINO

// Notice: don't use pin 6. That is LED_BUILTIN for the MKR1310
#define ROCK_PI_SHUTDOWN_PIN A1  // OUTPUT. This pin is driven high to command a shutdown in the rock pi
// Connected to ROCK PI S pin 12

#define ROCK_PI_MONITOR_PIN  A2  // INPUT pull-up. This is driven low whenever the rock pi is alive
// Connected to ROCK PI S pin 11

#define MOSFET_CONTROL_PIN  A3  // OUTPUT Mosfeot is switched on when this pin is high

#endif // ARDUINO

#endif //ACOUSEA_MKR1310_NODES_PIPOWERCONTROLPINS_HPP
