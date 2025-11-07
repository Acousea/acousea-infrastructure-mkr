#ifndef ACOUSEA_INFRASTRUCTURE_MKR_LAMBDATASK_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_LAMBDATASK_HPP


#include "ITask.hpp"
#include <utility>

template <typename Callable>
struct LambdaTask final : ITask
{
    Callable func;

    explicit LambdaTask(unsigned long interval, Callable&& f)
        : func(std::forward<Callable>(f))
    {
        this->interval = interval;
        this->lastTime = 0;
    }

    void execute() override { func(); }
};

template<typename Callable>
LambdaTask(unsigned long, Callable) -> LambdaTask<Callable>;

#endif //ACOUSEA_INFRASTRUCTURE_MKR_LAMBDATASK_HPP
