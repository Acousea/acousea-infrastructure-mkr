#ifndef ACOUSEA_INFRASTRUCTURE_MKR_ITASK_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_ITASK_HPP

struct ITask
{
    // antes TaskBase
    unsigned long interval{};
    unsigned long lastTime{};

    virtual void execute() = 0;

    virtual ~ITask() = default;
};
#endif //ACOUSEA_INFRASTRUCTURE_MKR_ITASK_HPP
