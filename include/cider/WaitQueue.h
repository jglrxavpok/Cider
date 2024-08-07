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
         *
         * 'lock' is used to synchronise accesses to the internal queue. Expected to be already locked when entering this function!
         */
        void suspendAndWait(UniqueSpinLock& lock, Cider::FiberHandle& toSuspend);

        /**
         * Pops the continuation at the start of the queue and executes it.
         */
        void notifyOne(UniqueSpinLock& lock);

        /**
         * Pops the continuation at the start of the queue and executes it.
         */
        void notifyAll(UniqueSpinLock& lock);

    private:
        std::deque<Cider::FiberHandle*> waitList;
    };
}