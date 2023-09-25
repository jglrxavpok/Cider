//
// Created by jglrxavpok on 23/09/2023.
//

#pragma once

#include <cider/internal/SpinLock.h>
#include <deque>

namespace Cider {
    class FiberHandle;

    /**
     * Queue of codes waiting for something (continuations).
     *
     * Intended usage is for locks
     */
    class WaitQueue {
    public:
        /**
         * Suspend the given fiber, which will be woken up later when it is popped from this queue.
         */
        void suspendAndWait(Cider::FiberHandle& toSuspend);

        /**
         * Pops the continuation at the start of the queue and executes it.
         * Thread-safe
         */
        void notifyOne();

        /**
         * Pops the continuation at the start of the queue and executes it.
         * Thread-safe
         */
        void notifyAll();

    private:
        SpinLock lock;
        std::deque<Cider::FiberHandle*> waitList;
    };
}