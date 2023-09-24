//
// Created by jglrxavpok on 23/09/2023.
//

#include <cider/Fiber.h>
#include <cider/scheduling/Scheduler.h>
#include <cider/scheduling/GreedyScheduler.h>
#include <cider/context.hpp>
#include <memory>
#include <utility>

namespace Cider {

    static char InvalidContextStack[64] = {0};

    static void FiberBottomOfCallstack() {
        fprintf(stderr, "Reached bottom of fiber callstack");
        exit(-1);
    }

    static Context InvalidContext = {
            .rip = (Register) FiberBottomOfCallstack,
            .rsp = (Register) InvalidContextStack,
    };

    static GreedyScheduler* defaultScheduler = new GreedyScheduler;

    Scheduler& getDefaultScheduler() {
        return *defaultScheduler;
    }

    Fiber::Fiber(Cider::FiberProc proc, void *userData, std::span<char> stack, Scheduler& scheduler)
    : scheduler(scheduler)
    , proc(proc)
    , pUserData(userData)
    , stack(stack)
    {
        parentContext = InvalidContext;
        currentContext = {0};
        currentContext.rip = (Register) FiberBottomOfCallstack;
        currentContext.rsp = (Register) (stack.data() + stack.size() - 8);

        swapContextOnTop(&parentContext, &currentContext, [&]() {
            FiberHandle handle;
            handle.pCurrentFiber = this;
            this->pFiberHandle = &handle;

            // setups stack with a fiber handle
            swap_context(&currentContext, &parentContext); // go back to constructor

            this->proc(*pFiberHandle, pUserData);
            swap_context(&currentContext, &parentContext); // execution finished: return to parent.
            // going past that line goes into FiberBottomOfCallstack
        });
    }

    static void stdFunctionProc(FiberHandle& fiber, void* pData) {
        auto pFunc = std::unique_ptr<StdFunctionFiberProc>((StdFunctionFiberProc*)pData); // will release std::function copy at end of scope
        (*pFunc)(fiber);
    }

    Fiber::Fiber(StdFunctionFiberProc proc, std::span<char> stack, Scheduler& scheduler)
    : Fiber(stdFunctionProc, new StdFunctionFiberProc(std::move(proc)) /* this instance will be deleted inside stdFunctionProc */, stack, scheduler) {}

    void Fiber::switchTo() {
        swap_context(&parentContext, &currentContext);
    }

    void Fiber::switchToWithOnTop(FiberProc onTopFunc, void* onTopUserData) {
        swapContextOnTop(&parentContext, &currentContext, [this, onTopFunc, onTopUserData]() {
            onTopFunc(*pFiberHandle, onTopUserData);
        });
    }

    void Fiber::switchToWithOnTop(StdFunctionFiberProc onTop) {
        swapContextOnTop(&parentContext, &currentContext, [this, onTopProc = std::move(onTop)]() {
            onTopProc(*pFiberHandle);
        });
    }

    FiberHandle* Fiber::getHandlePtr() {
        return pFiberHandle;
    }

    void FiberHandle::yield() {
        swap_context(&pCurrentFiber->currentContext, &pCurrentFiber->parentContext);
    }

    void FiberHandle::yieldOnTop(Proc onTopProc, void* onTopUserData) {
        swapContextOnTop(&pCurrentFiber->currentContext, &pCurrentFiber->parentContext, [this, onTopProc, onTopUserData]() {
            onTopProc(onTopUserData);
        });
    }

    void FiberHandle::yieldOnTop(StdFunctionProc onTop) {
        swapContextOnTop(&pCurrentFiber->currentContext, &pCurrentFiber->parentContext, [this, onTopProc = std::move(onTop)]() {
            onTopProc();
        });
    }

    void FiberHandle::resume() {
        pCurrentFiber->switchTo();
    }

    void FiberHandle::resumeOnTop(FiberProc onTopProc, void* onTopUserData) {
        pCurrentFiber->switchToWithOnTop(onTopProc, onTopUserData);
    }

    void FiberHandle::resumeOnTop(const StdFunctionFiberProc& onTop) {
        pCurrentFiber->switchToWithOnTop(onTop);
    }

    void FiberHandle::wake() {
        pCurrentFiber->scheduler.schedule(*this);
    }

    void FiberHandle::wake(Proc proc, void* userData) {
        pCurrentFiber->scheduler.schedule(*this, proc, userData);
    }

    void FiberHandle::wake(const StdFunctionProc& proc) {
        pCurrentFiber->scheduler.schedule(*this, proc);
    }
} // Cider