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
        while(acquired.test_and_set() /* while already locked */) {
            // mutex is already locked, ask to be woken up once lock is free
            waitQueue.suspendAndWait(fiber);
            // will yield until lock is free, and next iteration will attempt to get the lock again
        }
    }

    void Mutex::blockingLock() {
        while(acquired.test_and_set() /* while already locked */) {
            // mutex is already locked, ask to be woken up once lock is free
            std::this_thread::yield();
            // will yield until lock is free, and next iteration will attempt to get the lock again
        }
    }

    void Mutex::unlock() {
        acquired.clear();
        waitQueue.notifyOne();
    }

    LockGuard::LockGuard(Cider::FiberHandle& fiber, Mutex& _mutex): mutex(_mutex) {
        mutex.lock(fiber);
    }

    LockGuard::~LockGuard() {
        mutex.unlock();
    }

    BlockingLockGuard::BlockingLockGuard(Mutex& _mutex): mutex(_mutex) {
        mutex.blockingLock();
    }

    BlockingLockGuard::~BlockingLockGuard() {
        mutex.unlock();
    }
}
