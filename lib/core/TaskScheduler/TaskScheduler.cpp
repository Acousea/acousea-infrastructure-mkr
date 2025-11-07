#include "TaskScheduler.h"

#include "WatchDog/WatchDogUtils.hpp"


void TaskScheduler::addTask(ITask* task)
{
    if (taskCount < MAX_TASKS) tasks[taskCount++] = task;
}

void TaskScheduler::run() const
{
    const unsigned long now = getMillis();

    WatchdogUtils::reset(); // Reset watchdog before running tasks

    for (size_t i = 0; i < taskCount; ++i)
    {
        if (now - tasks[i]->lastTime >= tasks[i]->interval)
        {
            tasks[i]->execute();
            tasks[i]->lastTime = now;
            WatchdogUtils::reset();
        }
    }
}
