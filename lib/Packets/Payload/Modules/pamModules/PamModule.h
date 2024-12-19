#ifndef ACOUSEA_MKR1310_NODES_PAMMODULE_H
#define ACOUSEA_MKR1310_NODES_PAMMODULE_H


#include "Payload/Modules/SerializableModule.h"

class PamModule : public SerializableModule {
public:

    PamModule(ModuleCode::TYPES type, const std::vector<uint8_t> &value, const std::string &serialNumber,
              const std::string &name);

    [[nodiscard]] const std::string &getSerialNumber() const;

    [[nodiscard]] const std::string &getName() const;

protected:
    std::string serialNumber;
    std::string name;


};


#endif //ACOUSEA_MKR1310_NODES_PAMMODULE_H
