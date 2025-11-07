#ifndef ACOUSEA_INFRASTRUCTURE_MKR_GSMCONFIG_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_GSMCONFIG_HPP
#include <cstdio>

struct GsmConfig
{
    const char* const pin;
    const char* const apn;
    const char* const user;
    const char* const pass;
    const char* const clientId;
    const char* const broker;
    const int port;
    const char* const certificate;
    const char* const privateKey;

    static constexpr const char* baseTopic = "acousea/nodes";

    char inputTopic[100]{};
    char outputTopic[100]{};

    GsmConfig(
        const char* pin_,
        const char* apn_,
        const char* user_,
        const char* pass_,
        const char* clientId_,
        const char* broker_,
        int port_,
        const char* certificate_,
        const char* privateKey_
    )
        : pin(pin_), apn(apn_), user(user_), pass(pass_), clientId(clientId_),
          broker(broker_), port(port_), certificate(certificate_), privateKey(privateKey_)
    {
        snprintf(inputTopic, sizeof(inputTopic), "%s/mt/%s", baseTopic, clientId_);
        snprintf(outputTopic, sizeof(outputTopic), "%s/mo/%s", baseTopic, clientId_);
    }
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_GSMCONFIG_HPP
