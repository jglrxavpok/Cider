//
// Created by jglrxavpok on 24/09/2023.
//

#pragma once

#include <functional>

namespace Cider {
    class FiberHandle;

    using Proc = void(*)(void* userData);
    using StdFunctionProc = std::function<void()>;

    /**
     * Base class for all schedulers.
     */
    class Scheduler {
    public:
        virtual ~Scheduler() = default;

        /**
         * Call to schedule the given fiber for execution
         * @param toSchedule the fiber to execute
         */
        virtual void schedule(Cider::FiberHandle& toSchedule) = 0;

        /**
         * Call to schedule the given fiber for execution, running the given code once scheduling is done
         * (that does not mean when the fiber executes!)
         * @param toSchedule the fiber to execute
         */
        virtual void schedule(Cider::FiberHandle& toSchedule, Proc proc, void* userData) = 0;

        /**
         * Call to schedule the given fiber for execution, running the given code once scheduling is done
         * (that does not mean when the fiber executes!)
         * @param toSchedule the fiber to execute
         */
        virtual void schedule(Cider::FiberHandle& toSchedule, const StdFunctionProc& proc) = 0;
    };
}