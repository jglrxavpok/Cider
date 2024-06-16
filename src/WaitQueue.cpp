//
// Created by jglrxavpok on 24/09/2023.
//

#include <mutex>
#include <cider/WaitQueue.h>
#include <cider/Fiber.h>

namespace Cider {
    void WaitQueue::suspendAndWait(SpinLock& lock, Cider::FiberHandle& toSuspend) {
        waitList.push_back(&toSuspend);
        toSuspend.yieldOnTop([&lock]() {
            lock.unlock();
        });
    }
    void WaitQueue::suspendAndWait(std::unique_lock<std::mutex>& lock, Cider::FiberHandle& toSuspend) {
        waitList.push_back(&toSuspend);
        toSuspend.yieldOnTop([&lock]() {
            lock.unlock();
        });
    }

    void WaitQueue::notifyOne() {
        if(waitList.empty()) {
          //  lock.unlock();
            return;
        }

        Cider::FiberHandle* nextToRun = waitList.front();
        waitList.pop_front();
        nextToRun->wake([]() {
            //lock.unlock();
        });
    }

    void WaitQueue::notifyAll() {
        while(!waitList.empty()) {
            Cider::FiberHandle* nextToRun = waitList.front();
            waitList.pop_front();
            nextToRun->wake([]() {
               // lock.unlock();
            });
            //lock.lock();
        }
        //lock.unlock();
    }
}