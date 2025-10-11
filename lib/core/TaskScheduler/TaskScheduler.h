#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H


#include "time/getMillis.hpp"
#include <cstddef>  // for size_t

struct ITask {
    // antes TaskBase
    unsigned long interval{};
    unsigned long lastTime{};

    virtual void execute() = 0;

    virtual ~ITask() = default;
};

struct FunctionTask final : ITask {
    void (*func)();
    void execute() override { func(); }

    FunctionTask(const unsigned long interval, void (*f)()) {
        this->interval = interval;
        this->lastTime = 0;
        this->func = f;
    }
};

template<typename T>
struct MethodTask final : ITask {
    T* instance;
    union {
        void (T::*method)();
        void (T::*methodConst)() const;
    };

    void execute() override {
        if (method) (instance->*method)();
        else if (methodConst) (instance->*methodConst)();
    }

    MethodTask(unsigned long interval, T* inst, void (T::*meth)()) {
        this->interval = interval;
        this->lastTime = 0;
        this->instance = inst;
        this->method = meth;
    }

    MethodTask(unsigned long interval, T* inst, void (T::*meth)() const) {
        this->interval = interval;
        this->lastTime = 0;
        this->instance = inst;
        this->method = nullptr;
        this->methodConst = meth;
    }
};

class TaskScheduler {

public:
    void addTask(ITask *task) {
        if (taskCount < MAX_TASKS) tasks[taskCount++] = task;
    }

    void run() const {
        const unsigned long now = getMillis();
        for (size_t i = 0; i < taskCount; ++i) {
            if (now - tasks[i]->lastTime >= tasks[i]->interval) {
                tasks[i]->execute();
                tasks[i]->lastTime = now;
            }
        }
    }

private:
    static constexpr size_t MAX_TASKS = 10;
    ITask* tasks[MAX_TASKS]{};
    size_t taskCount = 0;
};

#endif // TASK_SCHEDULER_H
