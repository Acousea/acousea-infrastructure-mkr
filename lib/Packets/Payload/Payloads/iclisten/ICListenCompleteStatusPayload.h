#ifndef ACOUSEA_MKR1310_NODES_ICLISTEN_COMPLETE_STATUS_PAYLOAD_H
#define ACOUSEA_MKR1310_NODES_ICLISTEN_COMPLETE_STATUS_PAYLOAD_H

#include "../../Modules/pamModules/ICListen/ICListenHF.h"
#include "../../Payload.h"

class ICListenCompleteStatusPayload : public Payload {
public:
    explicit ICListenCompleteStatusPayload(const ICListenHF &icListenHF);

    // Serializa el payload a un vector de bytes
    [[nodiscard]] std::vector<uint8_t> toBytes() const override;

    [[nodiscard]] uint16_t getBytesSize() const override;

private:
    ICListenHF icListenHF;
};

#endif //ACOUSEA_MKR1310_NODES_ICLISTEN_COMPLETE_STATUS_PAYLOAD_H
