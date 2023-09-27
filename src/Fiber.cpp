//
// Created by jglrxavpok on 23/09/2023.
//

#include <cider/Fiber.h>
#include <cider/scheduling/Scheduler.h>
#include <cider/scheduling/GreedyScheduler.h>
#include <cider/context.hpp>
#include <memory>
#include <utility>

// Based on https://graphitemaster.github.io/fibers/#debuggers
// These macros are defined by GCC and/or clang
#ifdef CIDER_ASAN
extern "C" {
    // Check out sanitizer/asan-interface.h in compiler-rt for documentation.
    void __sanitizer_start_switch_fiber(void **fake_stack_save, const void *bottom, size_t size);
    void __sanitizer_finish_switch_fiber(void *fake_stack_save, const void **bottom_old, size_t *size_old);
}
#endif

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


    static void swapContextInternal(Context* current, Context* switchTo, std::span<char> stack, std::function<void()> onTop) {
#ifdef CIDER_ASAN
        if(stack.empty()) {
            // for swaps to non-fiber code
            __sanitizer_start_switch_fiber(nullptr, nullptr, 0);
            swapContextOnTop(current, switchTo, [=]() {
                __sanitizer_finish_switch_fiber(nullptr, nullptr, nullptr);
                onTop();
            });
        } else {
            void* fakeStackSave = nullptr;
            __sanitizer_start_switch_fiber(&fakeStackSave, stack.data(), stack.size());
            swapContextOnTop(current, switchTo, [=]() {
                __sanitizer_finish_switch_fiber(fakeStackSave, nullptr, nullptr);
                onTop();
            });
        }
#else
        swapContextOnTop(current, switchTo, [=]() {
            onTop();
        });
#endif
    }

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

        currentContext.rsp = currentContext.rsp & ~0xFull;

        swapContextInternal(&parentContext, &currentContext, stack, [&]() {
            FiberHandle handle;
            handle.pCurrentFiber = this;
            this->pFiberHandle = &handle;

            // setups stack with a fiber handle
            handle.yield(); // go back to constructor

            this->proc(*pFiberHandle, pUserData);
            handle.yield(); // execution finished: return to parent.
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
        swapContextInternal(&parentContext, &currentContext, stack, [](){});
    }

    void Fiber::switchToWithOnTop(FiberProc onTopFunc, void* onTopUserData) {
        swapContextInternal(&parentContext, &currentContext, stack, [this, onTopFunc, onTopUserData]() {
            onTopFunc(*pFiberHandle, onTopUserData);
        });
    }

    void Fiber::switchToWithOnTop(StdFunctionFiberProc onTop) {
        swapContextInternal(&parentContext, &currentContext, stack, [this, onTopProc = std::move(onTop)]() {
            onTopProc(*pFiberHandle);
        });
    }

    FiberHandle* Fiber::getHandlePtr() {
        return pFiberHandle;
    }

    void FiberHandle::yield() {
        swapContextInternal(&pCurrentFiber->currentContext, &pCurrentFiber->parentContext, {}, [](){});
    }

    void FiberHandle::yieldOnTop(Proc onTopProc, void* onTopUserData) {
        swapContextInternal(&pCurrentFiber->currentContext, &pCurrentFiber->parentContext, {}, [this, onTopProc, onTopUserData]() {
            onTopProc(onTopUserData);
        });
    }

    void FiberHandle::yieldOnTop(StdFunctionProc onTop) {
        swapContextInternal(&pCurrentFiber->currentContext, &pCurrentFiber->parentContext, {}, [this, onTopProc = std::move(onTop)]() {
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