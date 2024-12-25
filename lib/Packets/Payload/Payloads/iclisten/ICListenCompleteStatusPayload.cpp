#include "ICListenCompleteStatusPayload.h"

ICListenCompleteStatusPayload::ICListenCompleteStatusPayload(const ICListenHF &icListenHF) : icListenHF(
        icListenHF) {}

std::vector<uint8_t> ICListenCompleteStatusPayload::toBytes() const {
    return icListenHF.toBytes();
}

uint16_t ICListenCompleteStatusPayload::getBytesSize() const {
    return icListenHF.toBytes().size();
}



