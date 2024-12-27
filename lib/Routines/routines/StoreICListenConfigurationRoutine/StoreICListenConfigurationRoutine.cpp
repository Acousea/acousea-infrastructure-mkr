#include "StoreICListenConfigurationRoutine.h"

StoreICListenConfigurationRoutine::StoreICListenConfigurationRoutine(ICListenService &icListenService)
    : IRoutine(getClassNameString()), icListenService(icListenService) {
}

Result<Packet> StoreICListenConfigurationRoutine::execute(const Packet &packet) {
    const SetICListenConfigurationPayload configurationPayload = packet.getPayloadAs<SetICListenConfigurationPayload>();
    const auto aspects = configurationPayload.getAspectsAsEnums();
    for (const auto aspect : aspects) {
        switch (aspect) {
            case SetICListenConfigurationPayload::Aspect::STATUS:
                icListenService.getCache()->storeICListenStatus(configurationPayload.getStatus());
                break;
            case SetICListenConfigurationPayload::Aspect::LOGGING:
                icListenService.getCache()->storeICListenLoggingConfig(configurationPayload.getLoggingConfig());
                break;
            case SetICListenConfigurationPayload::Aspect::STREAMING:
                icListenService.getCache()->storeICListenStreamingConfig(configurationPayload.getStreamingConfig());
                break;
            case SetICListenConfigurationPayload::Aspect::STATS:
                icListenService.getCache()->storeICListenRecordingStats(configurationPayload.getRecordingStats());
                break;
        }
    }

    return Result<Packet>::emptySuccess();
}


