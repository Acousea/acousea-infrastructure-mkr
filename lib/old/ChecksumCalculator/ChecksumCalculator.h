#include <Arduino.h>

class ChecksumCalculator {
public:
    static uint8_t compute(const uint8_t *msg, uint8_t numbytes) {
        uint8_t chksum = 0;
        while (numbytes--) chksum += *msg++;
        return 0xFF - chksum;
    }
};
