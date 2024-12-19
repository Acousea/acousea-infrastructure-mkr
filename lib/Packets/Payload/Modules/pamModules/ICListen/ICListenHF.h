#ifndef ACOUSEA_MKR1310_NODES_ICLISTENHF_H
#define ACOUSEA_MKR1310_NODES_ICLISTENHF_H


#include "Payload/Modules/pamModules/PamModule.h"
#include "Payload/Modules/pamModules/ICListen/implementation/status/ICListenStatus.h"
#include "Payload/Modules/pamModules/ICListen/implementation/logging/ICListenLogigngConfig.h"
#include "Payload/Modules/pamModules/ICListen/implementation/streaming/ICListenStreamingConfig.h"
#include "Payload/Modules/pamModules/ICListen/implementation/stats/ICListenRecordingStats.h"

class ICListenHF : public PamModule {
public:
    ICListenHF(const ICListenStatus &status, const ICListenLoggingConfig &loggingConfig,
               const ICListenStreamingConfig &streamingConfig, const ICListenRecordingStats &recordingStats);

private:
    ICListenStatus status;
    ICListenLoggingConfig loggingConfig;
    ICListenStreamingConfig streamingConfig;
    ICListenRecordingStats recordingStats;
};

#endif //ACOUSEA_MKR1310_NODES_ICLISTENHF_H
