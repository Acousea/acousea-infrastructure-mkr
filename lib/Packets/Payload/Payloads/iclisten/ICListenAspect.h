#ifndef ICLISTENASPECT_H
#define ICLISTENASPECT_H

#include <cstdint>
#include <string>
#include <vector>
#include <bitset>

class ICListenAspect final {
public:
    // Enum para especificar los aspectos
    enum class Aspect : uint8_t {
        STATUS = 0x01,
        LOGGING = 0x02,
        STREAMING = 0x04,
        STATS = 0x08
    };

    // Convierte un aspecto en una string
    static std::string aspectToString(Aspect aspect);

    // Convierte un conjunto de aspectos en un vector de enums
    static std::vector<Aspect> getAspectsAsEnums(const std::bitset<8> &selectedAspects);

    // Convierte un conjunto de aspectos en un vector de strings
    static std::vector<std::string> getAspectsAsStrings(const std::bitset<8> &selectedAspects);

    // Join para los aspectos como strings
    static std::string joinAspectsAsString(const std::bitset<8> &selectedAspects, const std::string &delimiter = ", ");
};


#endif //ICLISTENASPECT_H
