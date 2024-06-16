//
// Created by jglrxavpok on 23/09/2023.
//

#include <gtest/gtest.h>
#include <cider/Fiber.h>
#include <cider/scheduling/Scheduler.h>
#include <cider/Mutex.h>
#include <deque>

TEST(FiberMutex, VerifyScheduling) {
    struct CustomScheduler: public Cider::Scheduler {
        std::deque<Cider::FiberHandle*> scheduled;

        void schedule(Cider::FiberHandle& toSchedule) override {
            scheduled.push_back(&toSchedule);
        }

        void schedule(Cider::FiberHandle& toSchedule, Cider::Proc proc, void* userData) override {
            scheduled.push_back(&toSchedule);
            proc(userData);
        }

        void schedule(Cider::FiberHandle& toSchedule, const Cider::StdFunctionProc& onTop) override {
            scheduled.push_back(&toSchedule);
            onTop();
        }

        void runOne() {
            Cider::FiberHandle* pHandle = scheduled.front();
            pHandle->resume();
            scheduled.pop_front();
        }
    };

    CustomScheduler scheduler;

    alignas(16) char stack1[4096];
    alignas(16) char stack2[4096];
    alignas(16) char stack3[4096];

    bool lockedForWrite = false;
    bool write1 = false;
    bool read2 = false;
    bool read3 = false;
    Cider::Mutex mutex;

    // Fiber 1 will lock a mutex
    // Fiber 2 & 3 will wait for the mutex to unlock
    auto fiberCode1 = [&](Cider::FiberHandle& fiber) {
        Cider::LockGuard g { fiber, mutex };
        lockedForWrite = true;
        fiber.yield(); // you should avoid this in real code! Locking mutexes and then yield is not the best idea out there
        write1 = true;
        lockedForWrite = false;
    };
    auto fiberCode2 = [&](Cider::FiberHandle& fiber) {
        Cider::LockGuard g { fiber, mutex };
        read2 = true;
    };
    auto fiberCode3 = [&](Cider::FiberHandle& fiber) {
        Cider::LockGuard g { fiber, mutex };
        read3 = true;
    };

    Cider::Fiber fiber1 { fiberCode1, std::span(stack1), scheduler };
    Cider::Fiber fiber2 { fiberCode2, std::span(stack2), scheduler };
    Cider::Fiber fiber3 { fiberCode3, std::span(stack3), scheduler };

    EXPECT_FALSE(lockedForWrite);
    EXPECT_FALSE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    fiber1.switchTo(); // Fiber 1 locks mutex, and then yield, nothing scheduled
    EXPECT_TRUE(lockedForWrite);
    EXPECT_FALSE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    fiber2.switchTo(); // Fiber 2 tries to lock mutex, fails and then waits on lock
    EXPECT_TRUE(lockedForWrite);
    EXPECT_FALSE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    fiber3.switchTo(); // Fiber 3 tries to lock mutex, fails and then waits on lock
    EXPECT_TRUE(lockedForWrite);
    EXPECT_FALSE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    fiber1.switchTo(); // Fiber 1 unlocks the mutex and schedules fiber 2 (the test scheduler is FIFO)
    EXPECT_FALSE(lockedForWrite);
    EXPECT_TRUE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    // Fiber 2 is scheduled
    EXPECT_EQ(scheduler.scheduled.size(), 1);
    EXPECT_EQ(scheduler.scheduled.front(), fiber2.getHandlePtr());

    // run fiber 2
    scheduler.runOne();
    EXPECT_FALSE(lockedForWrite);
    EXPECT_TRUE(write1);
    EXPECT_TRUE(read2);
    EXPECT_FALSE(read3);

    // Fiber 3 is scheduled
    EXPECT_EQ(scheduler.scheduled.size(), 1);
    EXPECT_EQ(scheduler.scheduled.front(), fiber3.getHandlePtr());

    // run fiber 3
    scheduler.runOne();
    EXPECT_FALSE(lockedForWrite);
    EXPECT_TRUE(write1);
    EXPECT_TRUE(read2);
    EXPECT_TRUE(read3);
    EXPECT_TRUE(scheduler.scheduled.empty());
}

TEST(FiberMutex, WithGreedyScheduler) {
    alignas(16) char stack1[5*4096];
    alignas(16) char stack2[5*4096];
    alignas(16) char stack3[5*4096];

    bool lockedForWrite = false;
    bool write1 = false;
    bool read2 = false;
    bool read3 = false;
    Cider::Mutex mutex;

    // Fiber 1 will lock a mutex
    // Fiber 2 & 3 will wait for the mutex to unlock
    auto fiberCode1 = [&](Cider::FiberHandle& fiber) {
        Cider::LockGuard g { fiber, mutex };
        lockedForWrite = true;
        fiber.yield(); // you should avoid this in real code! Locking mutexes and then yield is not the best idea out there
        write1 = true;
        lockedForWrite = false;
    };
    auto fiberCode2 = [&](Cider::FiberHandle& fiber) {
        Cider::LockGuard g { fiber, mutex };
        read2 = true;
    };
    auto fiberCode3 = [&](Cider::FiberHandle& fiber) {
        Cider::LockGuard g { fiber, mutex };
        read3 = true;
    };

    Cider::Fiber fiber1 { fiberCode1, std::span(stack1) };
    Cider::Fiber fiber2 { fiberCode2, std::span(stack2) };
    Cider::Fiber fiber3 { fiberCode3, std::span(stack3) };

    EXPECT_FALSE(lockedForWrite);
    EXPECT_FALSE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    fiber1.switchTo(); // Fiber 1 locks mutex, and then yield
    EXPECT_TRUE(lockedForWrite);
    EXPECT_FALSE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    fiber2.switchTo(); // Fiber 2 tries to lock mutex, fails and then waits on lock
    EXPECT_TRUE(lockedForWrite);
    EXPECT_FALSE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    fiber3.switchTo(); // Fiber 3 tries to lock mutex, fails and then waits on lock
    EXPECT_TRUE(lockedForWrite);
    EXPECT_FALSE(write1);
    EXPECT_FALSE(read2);
    EXPECT_FALSE(read3);

    fiber1.switchTo(); // Fiber 1 resumes, unlocking the mutex, which in turn wakes the fibers 2 & 3
    EXPECT_FALSE(lockedForWrite);
    EXPECT_TRUE(write1);
    EXPECT_TRUE(read2);
    EXPECT_TRUE(read3);
}