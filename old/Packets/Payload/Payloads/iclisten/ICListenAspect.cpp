#include "ICListenAspect.h"

std::string ICListenAspect::aspectToString(Aspect aspect) {
    switch (aspect) {
        case Aspect::STATUS: return "STATUS";
        case Aspect::LOGGING: return "LOGGING";
        case Aspect::STREAMING: return "STREAMING";
        case Aspect::STATS: return "STATS";
    }
    return "UNKNOWN"; // Default case to handle invalid values
}

std::vector<ICListenAspect::Aspect> ICListenAspect::getAspectsAsEnums(const std::bitset<8> &selectedAspects) {
    std::vector<Aspect> aspectsList;

    if (selectedAspects.to_ulong() & static_cast<uint8_t>(Aspect::STATUS)) {
        aspectsList.push_back(Aspect::STATUS);
    }
    if (selectedAspects.to_ulong() & static_cast<uint8_t>(Aspect::LOGGING)) {
        aspectsList.push_back(Aspect::LOGGING);
    }
    if (selectedAspects.to_ulong() & static_cast<uint8_t>(Aspect::STREAMING)) {
        aspectsList.push_back(Aspect::STREAMING);
    }
    if (selectedAspects.to_ulong() & static_cast<uint8_t>(Aspect::STATS)) {
        aspectsList.push_back(Aspect::STATS);
    }

    return aspectsList;
}

std::vector<std::string> ICListenAspect::getAspectsAsStrings(const std::bitset<8>& selectedAspects) {
    std::vector<std::string> aspectsList;
    for (size_t i = 0; i < selectedAspects.size(); ++i) {
        if (selectedAspects.test(i)) {
            aspectsList.push_back(aspectToString(static_cast<Aspect>(1 << i)));
        }
    }
    return aspectsList;
}

std::string ICListenAspect::joinAspectsAsString(const std::bitset<8>& selectedAspects, const std::string& delimiter) {
    const auto aspectsStrings = getAspectsAsStrings(selectedAspects);
    std::string joinedString;
    for (size_t i = 0; i < aspectsStrings.size(); ++i) {
        joinedString += aspectsStrings[i];
        if (i != aspectsStrings.size() - 1) {
            joinedString += delimiter;
        }
    }
    return joinedString;
}
