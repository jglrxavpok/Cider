//
// Created by jglrxavpok on 23/09/2023.
//

#include <cider/Mutex.h>
#include <cassert>
#include <thread>

namespace Cider {
    Mutex::Mutex() {
        acquired.clear();
    }

    Mutex::~Mutex() {
        assert(!acquired.test()); // someone is still waiting on this lock!
    }

    void Mutex::lock(Cider::FiberHandle& fiber) {
        while(true) {
            UniqueSpinLock g { waitQueueLock };

            // try locking
            if(!acquired.test_and_set(std::memory_order_acquire)) {
                return;
            }

            // mutex is already locked, ask to be woken up once lock is free
            waitQueue.suspendAndWait(g, fiber);
            // will yield until lock is free, and next iteration will attempt to get the lock again
        }
    }

    void Mutex::blockingLock() {
        while(acquired.test_and_set(std::memory_order_acquire) /* while already locked */) {
            // mutex is already locked, ask to be woken up once lock is free
            std::this_thread::yield();
            // will yield until lock is free, and next iteration will attempt to get the lock again
        }
    }

    void Mutex::unlock() {
        UniqueSpinLock g { waitQueueLock };
        acquired.clear(std::memory_order_release);
        waitQueue.notifyOne(g);
    }

    LockGuard::LockGuard(Cider::FiberHandle& fiber, Mutex& _mutex): mutex(_mutex) {
        mutex.lock(fiber);
    }

    LockGuard::~LockGuard() {
        mutex.unlock();
    }

    BlockingMutexGuard::BlockingMutexGuard(Mutex& _mutex): mutex(_mutex) {
        mutex.blockingLock();
    }

    BlockingMutexGuard::~BlockingMutexGuard() {
        mutex.unlock();
    }

    BlockingLockGuard::BlockingLockGuard(SpinLock& _lock): lock(_lock) {
        lock.lock();
    }

    BlockingLockGuard::~BlockingLockGuard() {
        lock.unlock();
    }
}
