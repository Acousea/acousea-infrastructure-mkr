#ifndef ACOUSEA_MKR1310_NODES_CLASSNAME_H
#define ACOUSEA_MKR1310_NODES_CLASSNAME_H

#define CLASS_NAME(name) \
    static const char* getClassNameCString() { return #name; } \
    static std::string getClassNameString() { return std::string(#name); }




#endif //ACOUSEA_MKR1310_NODES_CLASSNAME_H
