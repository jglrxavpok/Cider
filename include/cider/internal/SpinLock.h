//
// Created by jglrxavpok on 24/09/2023.
//

#pragma once

#include <atomic>
#include <cider/internal/config.h>

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

    /**
     * Equivalent of std::unique_lock but for SpinLock
     * The main difference with LockGuard & BlockingLockGuard is that destroying this object only unlocks if the lock was owned.
     * (Necessary to make WaitQueue work)
     */
    class UniqueSpinLock {
    public:
        explicit UniqueSpinLock(SpinLock& lock): lockRef(lock) {
            lockRef.lock();
            owning = true;
        }

        void lock() {
            CIDER_ASSERT(!owning, "UniqueSpinLock must not already own lock when calling lock explicitely");
            lockRef.lock();
            owning = true;
        }

        void unlock() {
            CIDER_ASSERT(owning, "UniqueSpinLock must already own lock when calling unlock");
            lockRef.unlock();
            owning = false;
        }

        ~UniqueSpinLock() {
            if(owning) {
                unlock();
            }
        }

    private:
        SpinLock& lockRef;
        bool owning = false;
    };
}
