//
// Created by jglrxavpok on 23/09/2023.
//

#include <cider/Fiber.h>
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

    Fiber::Fiber(Cider::FiberProc proc, void *userData, std::span<char> stack): proc(proc), pUserData(userData), stack(stack) {
        parentContext = InvalidContext;
        currentContext = {0};
        currentContext.rip = (Register) FiberBottomOfCallstack;
        currentContext.rsp = (Register) (stack.data() + stack.size() - 8);

        swapContextOnTop(&parentContext, &currentContext, [&]() {
            // set-ups stack with a fiber handle
            FiberHandle handle;
            handle.pCurrentFiber = this;

            swap_context(&currentContext, &parentContext); // go back to constructor

            this->proc(handle, pUserData);
            swap_context(&currentContext, &parentContext); // execution finished: return to parent.
            // going past that line goes into FiberBottomOfCallstack
        });
    }

    static void stdFunctionProc(FiberHandle& fiber, void* pData) {
        auto pFunc = std::unique_ptr<StdFunctionProc>((StdFunctionProc*)pData); // will release std::function copy at end of scope
        (*pFunc)(fiber);
    }

    Fiber::Fiber(StdFunctionProc proc, std::span<char> stack)
    : Fiber(stdFunctionProc, new StdFunctionProc(std::move(proc)) /* this instance will be deleted inside stdFunctionProc */, stack) {}

    void Fiber::switchTo() {
        swap_context(&parentContext, &currentContext);
    }

    void Fiber::switchToWithOnTop(FiberProc onTopFunc, void* onTopUserData) {
        swapContextOnTop(&parentContext, &currentContext, [this, onTopFunc, onTopUserData]() {
            FiberHandle handle;
            handle.pCurrentFiber = this;

            onTopFunc(handle, onTopUserData);
        });
    }

    void Fiber::switchToWithOnTop(StdFunctionProc onTop) {
        swapContextOnTop(&parentContext, &currentContext, [this, onTopProc = std::move(onTop)]() {
            FiberHandle handle;
            handle.pCurrentFiber = this;

            onTopProc(handle);
        });
    }

    void FiberHandle::yield() {
        swap_context(&pCurrentFiber->currentContext, &pCurrentFiber->parentContext);
    }
} // Cider