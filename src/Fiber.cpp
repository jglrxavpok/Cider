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

#ifdef _WIN64
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace Cider {
    /*static*/ void (*Fiber::OnFiberEnter)(Fiber *) = nullptr;

    /*static*/ void (*Fiber::OnFiberExit)(Fiber *) = nullptr;

    static char InvalidContextStack[64] = {0};

    static void FiberBottomOfCallstack() {
        fprintf(stderr, "Reached bottom of fiber callstack");
        exit(-1);
    }

    static Context InvalidContext = {
            .rip = (Register) FiberBottomOfCallstack,
            .rsp = (Register) InvalidContextStack,
    };

    static GreedyScheduler *defaultScheduler = new GreedyScheduler;


    Scheduler& getDefaultScheduler() {
        return *defaultScheduler;
    }

    Fiber::Fiber(Cider::FiberProc proc, void *userData, std::span<char> stack, Scheduler& scheduler)
            : scheduler(scheduler), proc(proc), pUserData(userData), stack(stack) {
        parentContext = InvalidContext;
        currentContext = {0};
        currentContext.rip = (Register) FiberBottomOfCallstack;
        currentContext.stackHighAddress = reinterpret_cast<Address>(stack.data() + stack.size());
        currentContext.stackLowAddress = reinterpret_cast<Address>(stack.data());
        currentContext.deallocationStack = reinterpret_cast<Address>(stack.data()-4096 /*TODO: real page size?*/);
        currentContext.guaranteedStackBytes = 0;

        currentContext.stackHighAddress = reinterpret_cast<Address>(stack.data() + stack.size());
        currentContext.rsp = currentContext.stackHighAddress;

        // TODO: check return address push, does not seem to work properly, lldb and PIX don't seem to find the proper bottom of stack
        currentContext.rsp -= 8*4; // parameter space

        // push return address
        currentContext.rsp -= 8; // return address
        *(Address*)(currentContext.rsp) = currentContext.rip;

        static_assert(sizeof(Context) == 272); // don't forget to change context_masm.asm if WinContext changes
        currentContext.rsp = currentContext.rsp & ~0xFull;

        currentContext.rsp -= sizeof(Context); // context to switch to
        *(Context*)currentContext.rsp = currentContext;

        swapContextInternalEntering(stack, [](void* pUserData) {
            auto* pThis = static_cast<Fiber*>(pUserData);
            FiberHandle handle;
            handle.pCurrentFiber = pThis;
            pThis->pFiberHandle = &handle;

            // setups stack with a fiber handle
            handle.yield(); // go back to constructor

            pThis->proc(*pThis->pFiberHandle, pThis->pUserData);
            handle.yield(); // execution finished: return to parent.
            // going past that line goes into FiberBottomOfCallstack
        }, this);
    }

    static void stdFunctionProc(FiberHandle& fiber, void *pData) {
        auto pFunc = std::unique_ptr<StdFunctionFiberProc>(
                (StdFunctionFiberProc *) pData); // will release std::function copy at end of scope
        (*pFunc)(fiber);
    }

    // avoid copies of std::function everywhere
    struct SwitchData {
        void* proc; // sometimes FiberProc, sometimes Proc
        void* userData;
        Fiber* pFiber;
    };

    Fiber::Fiber(StdFunctionFiberProc proc, std::span<char> stack, Scheduler& scheduler)
            : Fiber(stdFunctionProc, new StdFunctionFiberProc(
            std::move(proc)) /* this instance will be deleted inside stdFunctionProc */, stack, scheduler) {}

    void Fiber::switchTo() {
        swapContextInternalEntering(stack, [](void*) {}, nullptr);
    }

    void Fiber::switchToWithOnTop(FiberProc onTopFunc, void *onTopUserData) {
        SwitchData sd;
        sd.proc = onTopFunc;
        sd.userData = onTopUserData;
        sd.pFiber = this;
        swapContextInternalEntering(stack, [](void* userData) {
            auto* sd = static_cast<SwitchData*>(userData);
            Fiber* pThis = sd->pFiber;
            static_cast<FiberProc>(sd->proc)(*pThis->pFiberHandle, sd->userData);
        }, &sd);
    }

    void Fiber::switchToWithOnTop(StdFunctionFiberProc onTop) {
        struct SwitchDataStd {
            StdFunctionFiberProc stdFunc;
            Fiber* pThis;
        } sd;
        sd.stdFunc = std::move(onTop);
        sd.pThis = this;
        swapContextInternalEntering(stack, [](void* pUserData) {
            SwitchDataStd sdCopy = *static_cast<SwitchDataStd*>(pUserData);
            sdCopy.stdFunc(*sdCopy.pThis->pFiberHandle);
        }, &sd);
    }

    FiberHandle *Fiber::getHandlePtr() {
        return pFiberHandle;
    }

    void FiberHandle::yield() {
        pCurrentFiber->swapContextInternalExiting({}, [](void*) {}, nullptr);
    }

    void FiberHandle::yieldOnTop(Proc onTopProc, void *onTopUserData) {
        SwitchData sd {
            .proc = onTopProc,
            .userData = onTopUserData,
            .pFiber = nullptr,
        };
        pCurrentFiber->swapContextInternalExiting({},
                                   [](void* pUserData) {
                                       auto* sd = static_cast<SwitchData*>(pUserData);
                                       static_cast<Proc>(sd->proc)(sd->userData);
                                   }, &sd);
    }

    void FiberHandle::yieldOnTop(StdFunctionProc onTop) {
        struct SwitchDataStd {
            StdFunctionProc stdFunc;
        } sd;
        sd.stdFunc = std::move(onTop);
        pCurrentFiber->swapContextInternalExiting({},
                                   [](void *pUserData) {
                                        SwitchDataStd sdCopy = *static_cast<SwitchDataStd*>(pUserData);
                                        sdCopy.stdFunc();
                                   }, &sd);
    }

    void FiberHandle::resume() {
        pCurrentFiber->switchTo();
    }

    void FiberHandle::resumeOnTop(FiberProc onTopProc, void *onTopUserData) {
        pCurrentFiber->switchToWithOnTop(onTopProc, onTopUserData);
    }

    void FiberHandle::resumeOnTop(const StdFunctionFiberProc& onTop) {
        pCurrentFiber->switchToWithOnTop(onTop);
    }

    void FiberHandle::wake() {
        pCurrentFiber->scheduler.schedule(*this);
    }

    void FiberHandle::wake(Proc proc, void *userData) {
        pCurrentFiber->scheduler.schedule(*this, proc, userData);
    }

    void FiberHandle::wake(const StdFunctionProc& proc) {
        pCurrentFiber->scheduler.schedule(*this, proc);
    }

    void Fiber::swapContextInternalEntering(std::span<char> stack, Proc onTop, void* onTopUserData) {
        if(OnFiberExit && getCurrentFiberTLS() != nullptr) {
            OnFiberExit(getCurrentFiberTLS());
        }
        pParent = getCurrentFiberTLS();

        SwitchData sd {
            .proc = onTop,
            .userData = onTopUserData,
            .pFiber = this,
        };

        swapContextInternal(&currentContext, stack, [](Context* fromContext, void* pUserData) {
            auto* sd = static_cast<SwitchData*>(pUserData);
            if(OnFiberEnter) {
                OnFiberEnter(sd->pFiber);
            }
            getCurrentFiberTLS() = sd->pFiber;
            sd->pFiber->parentContext = *fromContext;
            // execute ontop function
            static_cast<Proc>(sd->proc)(sd->userData);
        }, &sd);
    }

    void Fiber::swapContextInternalExiting(std::span<char> stack, Proc onTop, void* onTopUserData) {
        if(OnFiberExit) {
            OnFiberExit(this);
        }

        SwitchData sd {
            .proc = onTop,
            .userData = onTopUserData,
            .pFiber = this,
        };
        swapContextInternal(&parentContext, stack, [](Context* fromContext, void* pUserData) {
            auto* sd = static_cast<SwitchData*>(pUserData);
            if(OnFiberEnter && sd->pFiber->pParent) {
                OnFiberEnter(sd->pFiber->pParent);
            }
            getCurrentFiberTLS() = sd->pFiber->pParent;
            sd->pFiber->currentContext = *fromContext;
            // execute ontop function
            static_cast<Proc>(sd->proc)(sd->userData);
        }, &sd);
    }

    void Fiber::swapContextInternal(Context *switchTo, std::span<char> stack, OnTopContextSwitchFunc onTop, void* onTopUserData) {
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
        SwitchData sd {
            .proc = onTop,
            .userData = onTopUserData,
            .pFiber = this,
        };
        swap_context_on_top(switchTo, &sd, [](Context* parentContext, void* pUserData) {
            auto* sd = static_cast<SwitchData*>(pUserData);
            auto* pFunc = static_cast<OnTopContextSwitchFunc>(sd->proc);
            pFunc(parentContext, sd->userData);
        });
#endif
    }
} // Cider