#ifndef IROUTINE_H
#define IROUTINE_H


#include <string>
#include "Result/Result.h"
#include <Packet.h>
#include "ClassName.h"

class IRoutineGeneric {
public:
    virtual ~IRoutineGeneric() = default;

    std::string routineName;

    explicit IRoutineGeneric(const std::string &name) : routineName(name) {}
    // Optionally include a method for execution without knowing the type.
};


struct VoidType {};

template<typename T>
class IRoutine : public IRoutineGeneric {
public:
    explicit IRoutine(const std::string &name) : IRoutineGeneric(name) {};

    virtual Result<Packet> execute(const T &input) = 0;

    // Virtual destructor para permitir la eliminación correcta de subclases
    virtual ~IRoutine() = default;
};

// Especialización parcial para VoidType
template<>
class IRoutine<VoidType> : public IRoutineGeneric {
public:
    explicit IRoutine(const std::string &name) : IRoutineGeneric(name) {};

    virtual Result<Packet> execute() = 0;

    virtual ~IRoutine() = default;
};

#endif // IROUTINE_H
