//
// Created by jglrxavpok on 24/09/2023.
//

#pragma once

#include <cider/scheduling/Scheduler.h>

namespace Cider {
    /**
     * Scheduler that immediately run a fiber that is scheduled.
     */
    class GreedyScheduler: public Scheduler {
    public:
        void schedule(FiberHandle& toSchedule) override;

        void schedule(FiberHandle& toSchedule, Proc proc, void *userData) override;

        void schedule(FiberHandle& toSchedule, const StdFunctionProc& proc) override;
    };
}