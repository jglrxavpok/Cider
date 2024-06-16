//
// Created by jglrxavpok on 24/09/2023.
//

#include <mutex>
#include <cider/WaitQueue.h>
#include <cider/Fiber.h>

namespace Cider {
    void WaitQueue::suspendAndWait(UniqueSpinLock& lock, Cider::FiberHandle& toSuspend) {
        waitList.push_back(&toSuspend);
        toSuspend.yieldOnTop([&lock]() {
            lock.unlock();
        });
    }

    void WaitQueue::notifyOne(UniqueSpinLock& lock) {
        if(waitList.empty()) {
            return;
        }

        Cider::FiberHandle* nextToRun = waitList.front();
        waitList.pop_front();
        nextToRun->wake([&lock]() {
            lock.unlock();
        });
    }

    void WaitQueue::notifyAll(UniqueSpinLock& lock) {
        while(!waitList.empty()) {
            Cider::FiberHandle* nextToRun = waitList.front();
            waitList.pop_front();
            nextToRun->wake([&lock]() {
                lock.unlock();
            });
            lock.lock();
        }
        lock.unlock();
    }
}