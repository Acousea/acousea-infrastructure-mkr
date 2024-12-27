#ifndef STOREICLISTENCONFIGURATIONROUTINE_H
#define STOREICLISTENCONFIGURATIONROUTINE_H

#include "IRoutine.h"
#include <ICListenService/ICListenService.h>

class StoreICListenConfigurationRoutine : public IRoutine<Packet> {
private:
    ICListenService &icListenService;

public:
    CLASS_NAME(SetNodeConfigurationRoutine)

    explicit StoreICListenConfigurationRoutine(ICListenService &icListenService);

    Result<Packet> execute(const Packet &packet) override;

};

#endif //STOREICLISTENCONFIGURATIONROUTINE_H
