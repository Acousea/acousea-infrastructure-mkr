#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "time/getMillis.hpp"
#include <cstddef>  // for size_t
#include "FunctionTask.hpp"
#include "MethodTask.hpp"
#include "Logger/Logger.h"


class TaskScheduler
{
public:
    void addTask(ITask* task);

    void run() const;

private:
    static constexpr size_t MAX_TASKS = 10;
    ITask* tasks[MAX_TASKS]{};
    size_t taskCount = 0;
};

#endif // TASK_SCHEDULER_H
