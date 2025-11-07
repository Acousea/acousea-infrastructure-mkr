#ifndef ACOUSEA_INFRASTRUCTURE_MKR_FUNCTIONTASK_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_FUNCTIONTASK_HPP

#include "ITask.hpp"

struct FunctionTask final : ITask
{
    void (*func)();
    void execute() override { func(); }

    FunctionTask(const unsigned long interval, void (*f)())
    {
        this->interval = interval;
        this->lastTime = 0;
        this->func = f;
    }
};

#endif //ACOUSEA_INFRASTRUCTURE_MKR_FUNCTIONTASK_HPP
