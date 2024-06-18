#include <Arduino.h>
class BitPacker {
public:
    static size_t pack(uint8_t *bitstr, size_t bit_pos, size_t bitstr_sz, uint32_t arg, size_t arg_sz) {
        size_t arg_pos;
        size_t bytePos = bit_pos / 8;
        size_t bitPos = bit_pos % 8;

        if ((bit_pos + arg_sz) > bitstr_sz) return (bit_pos + arg_sz);

        uint8_t *byteStr = bitstr + bytePos;
        for (arg_pos = 0; arg_pos < arg_sz; arg_pos++) {
            if (getBit(arg, arg_pos)) *byteStr = setBit(*byteStr, bitPos);
            else *byteStr = clearBit(*byteStr, bitPos);
            bitPos++;

            if (bitPos == 8) {
                byteStr++;
                bitPos = 0;
            }
        }
        return bit_pos + arg_sz;
    }

    static size_t unpack(const uint8_t *bitstr, size_t bit_pos, size_t bitstr_sz, uint32_t *arg, size_t arg_sz) {
        size_t arg_pos;
        size_t bytePos = bit_pos / 8;
        size_t bitPos = bit_pos % 8;

        if ((bit_pos + arg_sz) > bitstr_sz) return (bit_pos + arg_sz);

        *arg = (uint32_t)0;
        uint8_t *byteStr = (uint8_t *)bitstr + bytePos;

        for (arg_pos = 0; arg_pos < arg_sz; arg_pos++) {
            if (getBit(*byteStr, bitPos)) *arg = setBit(*arg, arg_pos);
            bitPos++;

            if (bitPos == 8) {
                byteStr++;
                bitPos = 0;
            }
        }
        return bit_pos + arg_sz;
    }

private:
    static uint32_t setBit(uint32_t w, uint32_t bitPos) {
        return (w | ((uint32_t)1 << bitPos));
    }

    static uint32_t getBit(uint32_t w, uint32_t bitPos) {
        return ((w >> bitPos) & (uint32_t)1);
    }

    static uint32_t clearBit(uint32_t w, uint32_t bitPos) {
        return (w & ~((uint32_t)1 << bitPos));
    }
};
