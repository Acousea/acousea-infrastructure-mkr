#include "StorageModule.h"

StorageModule StorageModule::from(int storageUsed, int storageTotal) {
    return {storageUsed, storageTotal};
}

StorageModule StorageModule::from(const std::vector<uint8_t> &value) {
    if (value.size() < sizeof(int) * 2) {
        ErrorHandler::handleError("Invalid value size for StorageModule");
    }
    return StorageModule(value);
}

int StorageModule::getStorageUsed() const {
    return storageUsed;
}

int StorageModule::getStorageTotal() const {
    return storageTotal;
}

StorageModule::StorageModule(const std::vector<uint8_t> &value)
        : SerializableModule(ModuleCode::TYPES::STORAGE, value) {
    if (VALUE.size() < sizeof(int) * 2) {
        ErrorHandler::handleError("Invalid value size for StorageModule");
    }
    std::memcpy(&storageUsed, VALUE.data(), sizeof(int));
    std::memcpy(&storageTotal, VALUE.data() + sizeof(int), sizeof(int));
}

StorageModule::StorageModule(int storageUsed, int storageTotal)
        : SerializableModule(ModuleCode::TYPES::STORAGE, serializeValues(storageUsed, storageTotal)),
          storageUsed(storageUsed), storageTotal(storageTotal) {}

std::vector<uint8_t> StorageModule::serializeValues(int storageUsed, int storageTotal) {
    std::vector<uint8_t> value(sizeof(int) * 2);
    std::memcpy(value.data(), &storageUsed, sizeof(int));
    std::memcpy(value.data() + sizeof(int), &storageTotal, sizeof(int));
    return value;
}
