#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include "ModuleProxy/ModuleProxy.hpp"

// Mock minimal de Router, para no depender de hardware
class DummyRouter {
public:
    static constexpr int broadcastAddress = 255;

    struct Sender {
        void sendSerial(const acousea_CommunicationPacket&) const {}
    };

    Sender sendFrom(int) const { return {}; }
};

TEST(ModuleProxyTest, CacheStoresAndRetrieves)
{
    DummyRouter router;
    ModuleProxy proxy(reinterpret_cast<Router&>(router));  // reinterpretación segura en tests

    // Crear un wrapper simulado
    acousea_ModuleWrapper wrapper = acousea_ModuleWrapper_init_default;
    wrapper.which_module = acousea_ModuleWrapper_icListenHF_tag;
    wrapper.module.icListenHF = acousea_ICListenHF_init_default;
    strcpy(wrapper.module.icListenHF.serialNumber, "TEST123");

    proxy.getCache().store(acousea_ModuleCode_ICLISTEN_HF, wrapper);

    auto retrieved = proxy.getCache().get(acousea_ModuleCode_ICLISTEN_HF);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(std::string(retrieved->module.icListenHF.serialNumber), "TEST123");
    EXPECT_TRUE(proxy.getCache().fresh(acousea_ModuleCode_ICLISTEN_HF));
}

TEST(ModuleProxyTest, InvalidateRemovesFreshness)
{
    DummyRouter router;
    ModuleProxy proxy(reinterpret_cast<Router&>(router));

    acousea_ModuleWrapper wrapper = acousea_ModuleWrapper_init_default;
    wrapper.which_module = acousea_ModuleWrapper_icListenHF_tag;
    wrapper.module.icListenHF = acousea_ICListenHF_init_default;

    proxy.getCache().store(acousea_ModuleCode_ICLISTEN_HF, wrapper);
    proxy.getCache().invalidate(acousea_ModuleCode_ICLISTEN_HF);

    EXPECT_FALSE(proxy.getCache().fresh(acousea_ModuleCode_ICLISTEN_HF));
}
