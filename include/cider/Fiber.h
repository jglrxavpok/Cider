//
// Created by jglrxavpok on 23/09/2023.
//

#pragma once
#include <span>
#include <functional>
#include <cider/context.h>

namespace Cider {
    class Fiber;

    class FiberHandle {
    public:
        // TODO: fiber local storage

        /**
         * Returns execution to parent fiber.
         * Can throw if an on-top function was added on top before resuming this fiber, and that on-top function throws
         */
        void yield();

    private:
        Fiber* pCurrentFiber = nullptr;

        friend class Fiber;
    };

    using FiberProc = void(*)(Cider::FiberHandle& fiber, void* userData);
    using StdFunctionProc = std::function<void(Cider::FiberHandle&)>;

    class Fiber {
    public:
        /**
         * Creates a new fiber, that will call 'proc' when switched to.
         * 'userData' is used to provide data to the fiber. User is responsible for freeing it up.
         * 'stack' represents some memory to use for the stack of that fiber.
         * @param proc function called when the fiber is switched into
         * @param userData data available to the fiber. User is responsible for freeing it up
         * @param stack memory to use for the stack of this fiber
         */
        Fiber(FiberProc proc, void* userData, std::span<char> stack);

        /**
         * Creates a new fiber, that will call 'proc' when switched to.
         * 'stack' represents some memory to use for the stack of that fiber.
         * @param proc function called when the fiber is switched into
         * @param stack memory to use for the stack of this fiber
         */
        Fiber(StdFunctionProc proc, std::span<char> stack);

        /**
         * Sets the current execution context to this fiber.
         * When the fiber yields or reaches the end of its 'proc' function (see constructor), the execution resumes
         * right after this call.
         *
         * Invalid to call if the fiber has already reached the end of its 'proc' function.
         */
        void switchTo();

        /**
         * Similar to 'switchTo', but executes the given function on top of the current fiber
         * @param onTop function to execute on top of current fiber callstack
         * @param onTopUserData user data for the on-top function. User is responsible for its lifetime.
         */
        void switchToWithOnTop(FiberProc onTop, void* onTopUserData);

        /**
         * Similar to 'switchTo', but executes the given function on top of the current fiber
         */
        void switchToWithOnTop(StdFunctionProc onTop);

        /**
         * Switches to this fiber, and throws a CancelledException through the fiber.
         */
        // TODO: requires exception support! void cancel();

    private:
        Cider::FiberProc proc; // function to execute
        void* pUserData = nullptr; // pointer to user data
        std::span<char> stack; // user provided stack

        Context parentContext; // context to resume to when execution stops or yields
        Context currentContext; // context to resume when switching into this fiber

        friend class FiberHandle;
    };

} // Cider
