#include "StoreICListenConfigurationRoutine.h"

StoreICListenConfigurationRoutine::StoreICListenConfigurationRoutine(ICListenService &icListenService)
    : IRoutine(getClassNameString()), icListenService(icListenService) {
}

Result<Packet> StoreICListenConfigurationRoutine::execute(const Packet &packet) {
    const SetICListenConfigurationPayload configurationPayload = packet.getPayloadAs<SetICListenConfigurationPayload>();
    const auto aspects = ICListenAspect::getAspectsAsEnums(configurationPayload.getAspects());
    Logger::logInfo(
        getClassNameString() +
        "Storing ICListen configuration aspects for aspects: " +
        ICListenAspect::joinAspectsAsString(configurationPayload.getAspects())
    );
    for (const auto aspect: aspects) {
        switch (aspect) {
            case ICListenAspect::Aspect::STATUS:
                icListenService.getCache()->storeICListenStatus(configurationPayload.getStatus());
                break;
            case ICListenAspect::Aspect::LOGGING:
                icListenService.getCache()->storeICListenLoggingConfig(configurationPayload.getLoggingConfig());
                break;
            case ICListenAspect::Aspect::STREAMING:
                icListenService.getCache()->storeICListenStreamingConfig(configurationPayload.getStreamingConfig());
                break;
            case ICListenAspect::Aspect::STATS:
                icListenService.getCache()->storeICListenRecordingStats(configurationPayload.getRecordingStats());
                break;
        }
    }

    return Result<Packet>::emptySuccess();
}
