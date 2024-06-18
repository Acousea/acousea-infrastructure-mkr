#include <Arduino.h>

class Emergency {
public:
    static void signal(int num_rep) {
        uint32_t delay_ms = 300;
        while(1) {
            for (int rep = 0; rep < num_rep; rep++) {
                digitalWrite(LED_BUILTIN, HIGH);
                delay(delay_ms);
                digitalWrite(LED_BUILTIN, LOW);
                delay(delay_ms);
            }
            delay_ms = (delay_ms == 300)?600:300;
            delay(1000);
        }
    }
};
