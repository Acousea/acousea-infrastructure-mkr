#ifndef ACOUSEA_MKR1310_NODES_ICLISTENHF_H
#define ACOUSEA_MKR1310_NODES_ICLISTENHF_H


#include "../../pamModules/PamModule.h"
#include "implementation/status/ICListenStatus.h"
#include "implementation/logging/ICListenLoggingConfig.h"
#include "implementation/streaming/ICListenStreamingConfig.h"
#include "implementation/stats/ICListenRecordingStats.h"

class ICListenHF : public PamModule {
public:
    ICListenHF(const ICListenStatus &status, const ICListenLoggingConfig &loggingConfig,
               const ICListenStreamingConfig &streamingConfig, const ICListenRecordingStats &recordingStats);

public:
    const ICListenStatus status;
    const ICListenLoggingConfig loggingConfig;
    const ICListenStreamingConfig streamingConfig;
    const ICListenRecordingStats recordingStats;

    static ICListenHF fromBytes(const std::vector<uint8_t> &data);

    std::string toString() const;
};

#endif //ACOUSEA_MKR1310_NODES_ICLISTENHF_H
