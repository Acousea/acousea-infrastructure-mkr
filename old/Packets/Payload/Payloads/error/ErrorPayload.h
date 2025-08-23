#ifndef ACOUSEA_MKR1310_NODES_ERRORPAYLOAD_H
#define ACOUSEA_MKR1310_NODES_ERRORPAYLOAD_H

#include "Routing/ErrorCode/ErrorCode.h"
#include "Payload/Payload.h"
#include "ErrorHandler/ErrorHandler.h"

class ErrorPayload : public Payload {
public:

    explicit ErrorPayload(ErrorCode errorCode);

    [[nodiscard]] std::vector<uint8_t> toBytes() const override;

    [[nodiscard]] uint16_t getBytesSize() const override;

    static ErrorPayload fromBytes(const std::vector<uint8_t> &data);

private:
    ErrorCode errorCode;
};

#endif //ACOUSEA_MKR1310_NODES_ERRORPAYLOAD_H
