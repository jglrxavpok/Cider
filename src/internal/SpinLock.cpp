//
// Created by jglrxavpok on 24/09/2023.
//

#include <cider/internal/config.h>
#include <cider/internal/SpinLock.h>
#include <cassert>
#include <thread>

namespace Cider {
    SpinLock::SpinLock(): acquired() {
        acquired.clear();
    }

    void SpinLock::lock() {
        while(!tryLock()) {
            std::this_thread::yield();
        }
    }

    bool SpinLock::tryLock() {
        bool alreadyLocked = acquired.test_and_set(std::memory_order_acquire);
        return !alreadyLocked;
    }

    void SpinLock::unlock() {
        acquired.clear(std::memory_order_release);
    }

    SpinLock::~SpinLock() {
        CIDER_ASSERT(!acquired.test(std::memory_order_acquire), "Lock should not be acquired when destroying it");
    }
}