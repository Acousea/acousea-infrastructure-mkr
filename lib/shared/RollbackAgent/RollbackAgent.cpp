#include "RollbackAgent.hpp"

#include "Logger/Logger.h"
#include <cstddef>

void RollbackAgent::registerAction(const Callback fn, void* ctx) noexcept
{
    if (count >= MAX_ACTIONS)
    {
        LOG_CLASS_WARNING("CommitAgent: Max actions reached (%zu)", MAX_ACTIONS);
        return;
    }
    actions[count++] = {fn, ctx};
}

void RollbackAgent::commit() noexcept
{
    for (size_t i = 0; i < count; ++i)
    {
        if (actions[i].fn)
            actions[i].fn(actions[i].ctx);
    }
    clear();
}

void RollbackAgent::clear() noexcept { count = 0; }


bool RollbackAgent::empty() const noexcept { return count == 0; }
