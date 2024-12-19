#ifndef ACOUSEA_MKR1310_NODES_STORAGEMODULE_H
#define ACOUSEA_MKR1310_NODES_STORAGEMODULE_H


#include "Payload/Modules/SerializableModule.h"

class StorageModule : public SerializableModule {
public:
    static StorageModule from(int storageUsed, int storageTotal);

    static StorageModule from(const std::vector<uint8_t> &value);


    [[nodiscard]] int getStorageUsed() const;

    [[nodiscard]] int getStorageTotal() const;

private:
    explicit StorageModule(const std::vector<uint8_t> &value);

    StorageModule(int storageUsed, int storageTotal);

    static std::vector<uint8_t> serializeValues(int storageUsed, int storageTotal);

private:
    int storageUsed;
    int storageTotal;
};

#endif 
