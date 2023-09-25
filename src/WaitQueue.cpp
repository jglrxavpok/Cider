//
// Created by jglrxavpok on 24/09/2023.
//

#include <cider/WaitQueue.h>
#include <cider/Fiber.h>

namespace Cider {
    void WaitQueue::suspendAndWait(Cider::FiberHandle& toSuspend) {
        lock.lock();
        waitList.push_back(&toSuspend);
        toSuspend.yieldOnTop([this]() {
            lock.unlock();
        });
    }

    void WaitQueue::notifyOne() {
        lock.lock();
        if(waitList.empty()) {
            lock.unlock();
            return;
        }

        Cider::FiberHandle* nextToRun = waitList.front();
        waitList.pop_front();
        nextToRun->wake([this]() {
            lock.unlock();
        });
    }

    void WaitQueue::notifyAll() {
        lock.lock();
        while(!waitList.empty()) {
            Cider::FiberHandle* nextToRun = waitList.front();
            waitList.pop_front();
            nextToRun->wake([this]() {
                lock.unlock();
            });
            lock.lock();
        }
        lock.unlock();
    }
}