//
// Created by jglrxavpok on 24/09/2023.
//

#include <cider/scheduling/GreedyScheduler.h>
#include <cider/Fiber.h>

namespace Cider {
    void GreedyScheduler::schedule(FiberHandle& toSchedule) {
        toSchedule.resume();
    }

    void GreedyScheduler::schedule(FiberHandle& toSchedule, Proc proc, void* userData) {
        toSchedule.resumeOnTop([proc, userData](FiberHandle& fiber) {
            proc(userData);
        });
    }

    void GreedyScheduler::schedule(FiberHandle& toSchedule, const StdFunctionProc& proc) {
        toSchedule.resumeOnTop([proc](FiberHandle& fiber) {
            proc();
        });
    }
}