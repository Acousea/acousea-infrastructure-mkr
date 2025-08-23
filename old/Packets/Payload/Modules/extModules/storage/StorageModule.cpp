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

    // Convertir de big endian a little endian si es necesario
    std::reverse(reinterpret_cast<char *>(&storageUsed), reinterpret_cast<char *>(&storageUsed) + sizeof(int));
    std::reverse(reinterpret_cast<char *>(&storageTotal), reinterpret_cast<char *>(&storageTotal) + sizeof(int));
}

StorageModule::StorageModule(int storageUsed, int storageTotal)
    : SerializableModule(ModuleCode::TYPES::STORAGE, serializeValues(storageUsed, storageTotal)),
      storageUsed(storageUsed), storageTotal(storageTotal) {
}

std::vector<uint8_t> StorageModule::serializeValues(int storageUsed, int storageTotal) {
    std::vector<uint8_t> value(sizeof(int) * 2);

    // Convertir a big endian si el sistema es little endian
    const auto storageUsedBytes = reinterpret_cast<char *>(&storageUsed);
    std::reverse(storageUsedBytes, storageUsedBytes + sizeof(int));
    const auto storageTotalBytes = reinterpret_cast<char *>(&storageTotal);
    std::reverse(storageTotalBytes, storageTotalBytes + sizeof(int));

    // Copiar los valores serializados al vector
    std::memcpy(value.data(), &storageUsed, sizeof(int));
    std::memcpy(value.data() + sizeof(int), &storageTotal, sizeof(int));

    return value;
}
