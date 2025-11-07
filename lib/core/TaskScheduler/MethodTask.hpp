#ifndef ACOUSEA_INFRASTRUCTURE_MKR_METHODTASK_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_METHODTASK_HPP

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

#endif //ACOUSEA_INFRASTRUCTURE_MKR_METHODTASK_HPP