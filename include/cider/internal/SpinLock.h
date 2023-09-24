//
// Created by jglrxavpok on 24/09/2023.
//

#pragma once

#include <atomic>

namespace Cider {
    /**
     * Simple lock that yields the current thread until the lock is free.
     * Intended for code paths with low contention
     */
    class SpinLock {
    public:
        explicit SpinLock();
        ~SpinLock();

        bool tryLock();
        void lock();
        void unlock();

        // not copiable nor moveable
        SpinLock(const SpinLock&) = delete;
        SpinLock(SpinLock&&) = delete;
        SpinLock& operator=(const SpinLock&) = delete;
        SpinLock& operator=(SpinLock&&) = delete;

    private:
        std::atomic_flag acquired;
    };
}