#include "ErrorPayload.h"

ErrorPayload::ErrorPayload(ErrorCode errorCode) : errorCode(errorCode) {}

std::vector<uint8_t> ErrorPayload::toBytes() const {
    return {static_cast<uint8_t>(errorCode)};
}

uint16_t ErrorPayload::getBytesSize() const {
    return 1;
}

ErrorPayload ErrorPayload::fromBytes(const std::vector<uint8_t> &data) {
    if (data.size() != 1) {
        ErrorHandler::handleError("Invalid data size for ErrorPayload");
//            throw std::invalid_argument("Invalid data size for ErrorPayload");
    }
    return ErrorPayload(static_cast<ErrorCode>(data[0]));
}
