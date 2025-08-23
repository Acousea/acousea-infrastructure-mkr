#include "FetchICListenConfigurationPayload.h"

// Constructor privado
FetchICListenConfigurationPayload::FetchICListenConfigurationPayload(const std::bitset<8>& selectedAspects)
    : selectedAspects(selectedAspects) {}

std::bitset<8> FetchICListenConfigurationPayload::getAspects() const {
    return selectedAspects;
}

// Tamaño en bytes del payload
uint16_t FetchICListenConfigurationPayload::getBytesSize() const {
    return 1; // Solo contiene un byte para los aspectos seleccionados
}

// Serialización del payload a un vector de bytes
std::vector<uint8_t> FetchICListenConfigurationPayload::toBytes() const {
    std::vector<uint8_t> bytes;
    bytes.push_back(static_cast<uint8_t>(selectedAspects.to_ulong()));
    return bytes;
}

FetchICListenConfigurationPayload FetchICListenConfigurationPayload::fromBytes(const std::vector<uint8_t> &data) {
    if (data.size() != 1) {
        ErrorHandler::handleError("Invalid data size for FetchICListenConfigurationPayload");
    }
    std::bitset<8> aspects(data[0]);
    return FetchICListenConfigurationPayload(aspects);
}

