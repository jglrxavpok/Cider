//
// Created by jglrxavpok on 23/09/2023.
//

#pragma once

#include <atomic>
#include <cider/Fiber.h>
#include <cider/WaitQueue.h>

namespace Cider {
    /**
     * Object used for mutual exclusion: prevent two pieces of code to execute concurrently.
     * This mutex is fiber-aware: you can make a fiber yield until the lock is free.
     * This mutex is NOT reentrant/recursive.
     * When the lock is freed, if there is a fiber waiting on this lock, it is immediately run!
     */
    class Mutex {
    public:
        explicit Mutex();
        ~Mutex();

        void lock(Cider::FiberHandle& fiber);
        void unlock();

    private:
        WaitQueue waitQueue;
        std::atomic_flag acquired;
    };

    /**
     * RAII construct to lock mutex at this object construction, and unlock on its destruction
     */
    class LockGuard {
    public:
        /**
         * Constructs a lock guard, and attempts to lock the given mutex
         * @param fiber fiber to yield if lock fails
         * @param mutex mutex to lock
         */
        explicit LockGuard(Cider::FiberHandle& fiber, Mutex& mutex);
        ~LockGuard();

    private:
        Mutex& mutex;
    };
}