#ifndef ACOUSEA_MKR1310_NODES_CLASSNAME_H
#define ACOUSEA_MKR1310_NODES_CLASSNAME_H

#include <string>

#define CLASS_NAME(name) \
static const char* getClassNameCString(bool withSuffix = true) { \
return withSuffix ? "[" #name "]: " : "[" #name "]"; \
} \
static std::string getClassNameString(bool withSuffix = true) { \
return withSuffix ? std::string("[" #name "]: ") : std::string("[" #name "]"); \
}

#endif //ACOUSEA_MKR1310_NODES_CLASSNAME_H
