//
// Created by jglrxavpok on 23/09/2023.
//

#pragma once
#include <span>
#include <functional>
#include <array>
#include <cider/context.h>

#if defined(__clang__)
    #if __has_feature(address_sanitizer)
        #define CIDER_ASAN
    #endif
#else
    #if defined(__SANITIZE_ADDRESS__)
        #define CIDER_ASAN
    #endif
#endif

namespace Cider {
    class Fiber;
    class FiberHandle;
    class Scheduler;

    using FiberProc = void(*)(Cider::FiberHandle& fiber, void* userData);
    using StdFunctionFiberProc = std::function<void(Cider::FiberHandle&)>;

    using Proc = void(*)(void* userData);
    using StdFunctionProc = std::function<void()>;

    Scheduler& getDefaultScheduler();

    class FiberHandle {
    public:
        /**
         * Local storage usable by fibers.
         * if you need more memory, put pointers in here
         */
        std::array<char, 64> localStorage{ 0 };

        /**
         * Returns execution to parent fiber.
         * Can throw if an on-top function was added on top before resuming this fiber, and that on-top function throws
         */
        void yield();

        /**
         * Returns execution to parent fiber BUT runs some code before resuming.
         * You are not guaranteed to come back to a fiber, so you cannot yield inside the code to run when resuming.
         */
        void yieldOnTop(Proc onTopProc, void* onTopUserData);

        /**
         * Returns execution to parent fiber BUT runs some code before resuming.
         * You are not guaranteed to come back to a fiber, so you cannot yield inside the code to run when resuming.
         */
        void yieldOnTop(StdFunctionProc onTop);

        /**
         * Immediately resumes execution of suspended fiber on the current thread.
         */
        void resume();

        /**
         * Immediately resumes execution of suspended fiber on the current thread, and runs the given code before resuming.
         */
        void resumeOnTop(FiberProc onTopProc, void* onTopUserData);

        /**
         * Immediately resumes execution of suspended fiber on the current thread, and runs the given code before resuming.
         */
        void resumeOnTop(const StdFunctionFiberProc& onTop);

        /**
         * Schedule this fiber for execution
         */
        void wake();

        /**
         * Schedule this fiber for execution, and runs the given code when the fiber is woken up
         */
        void wake(Proc proc, void* userData);

        /**
         * Schedule this fiber for execution, and runs the given code when the fiber is woken up
         */
        void wake(const StdFunctionProc& proc);

    private:
        Fiber* pCurrentFiber = nullptr;

        friend class Fiber;
    };

    class Fiber {
    public:
        using FiberFrontierCallback = void (*)(Fiber*);

        /**
         * If not nullptr, this callback is called when code enters a fiber,
         *  no matter how (starting, yielding to another, or finishing a fiber and returning to the parent).
         */
        static FiberFrontierCallback OnFiberEnter;

        /**
         * If not nullptr, this callback is called when code exits a fiber,
         *  no matter how (finishing, yielding to another, or waiting).
         */
        static FiberFrontierCallback OnFiberExit;

        /**
         * Creates a new fiber, that will call 'proc' when switched to.
         * 'userData' is used to provide data to the fiber. User is responsible for freeing it up.
         * 'stack' represents some memory to use for the stack of that fiber.
         * @param proc function called when the fiber is switched into
         * @param userData data available to the fiber. User is responsible for freeing it up
         * @param stack memory to use for the stack of this fiber
         * @param pScheduler pointer to the scheduler to use for this fiber, by default 'getDefaultScheduler()'
         */
        Fiber(FiberProc proc, void* userData, std::span<char> stack, Scheduler& scheduler = getDefaultScheduler());

        /**
         * Creates a new fiber, that will call 'proc' when switched to.
         * 'stack' represents some memory to use for the stack of that fiber.
         * @param proc function called when the fiber is switched into
         * @param stack memory to use for the stack of this fiber
         * @param pScheduler pointer to the scheduler to use for this fiber, by default 'getDefaultScheduler()'
         */
        Fiber(StdFunctionFiberProc proc, std::span<char> stack, Scheduler& scheduler = getDefaultScheduler());

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
        void switchToWithOnTop(StdFunctionFiberProc onTop);

        /**
         * Switches to this fiber, and throws a CancelledException through the fiber.
         */
        // TODO: requires exception support! void cancel();

        Cider::FiberHandle* getHandlePtr();

    private:
        Scheduler& scheduler;
        Cider::FiberProc proc; // function to execute
        void* pUserData = nullptr; // pointer to user data
        std::span<char> stack; // user provided stack
        FiberHandle* pFiberHandle = nullptr; // fiber handle, stored inside stack directly
        Fiber* pParent = nullptr;

        Context parentContext; // context to resume to when execution stops or yields
        Context currentContext; // context to resume when switching into this fiber

        void swapContextInternalEntering(std::span<char> stack, Proc onTop, void* onTopUserData);
        void swapContextInternalExiting(std::span<char> stack, Proc onTop, void* onTopUserData);

        using OnTopContextSwitchFunc = void(*)(Context*, void* pUserData);
        void swapContextInternal(Context* switchTo, std::span<char> stack, OnTopContextSwitchFunc pOnTop, void* onTopUserData);

#ifdef CIDER_ASAN

#endif

        static Fiber*& getCurrentFiberTLS();

        friend class FiberHandle;
    };

} // Cider
