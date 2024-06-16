//
// Created by jglrxavpok on 22/09/2023.
//

#include <gtest/gtest.h>

#include <cider/context.h>

std::atomic<bool> subFunctionStep1 = false;
std::atomic<bool> subFunctionStep2 = false;

static Context parentContext {0};
static Context subfunctionInnerContext {0};

static void make_context(Context* out, char* stack_top, char* stack_bottom, void (*function_pointer)()) {
    Address rsp = (Address)stack_top;
    rsp -= sizeof(Context);
    rsp -= 8; // "return" address
    rsp -= 4*8; // parameter space?
    rsp = rsp & ~0xFull;

    *out = {0};
    out->rsp = (Address)rsp;
    out->rip = (Register)function_pointer;
    out->stackHighAddress = (Address)stack_top;
    out->stackLowAddress = (Address)stack_bottom;
    out->deallocationStack = (Address)stack_bottom;
    out->guaranteedStackBytes = 0;
    memcpy((char*)rsp, out, sizeof(Context));
}

static void subFunction() {
    EXPECT_FALSE(subFunctionStep1);
    subFunctionStep1 = true;

    // yield execution to parent
    swap_context_on_top(&parentContext, nullptr, [](Context* parentContext, void*){subfunctionInnerContext = *parentContext;});

    EXPECT_FALSE(subFunctionStep2);
    subFunctionStep2 = true;

    swap_context_on_top(&parentContext, nullptr, [](Context* parentContext,void*){subfunctionInnerContext = *parentContext;});
    FAIL(); // will never execute
}

TEST(SwapContext, SwapYieldAndContinue) {
    EXPECT_FALSE(subFunctionStep1);
    EXPECT_FALSE(subFunctionStep2);

    // setup new context
    alignas(16) char stack[4096*1] = {0};
    Context subFunctionContext;
    make_context(&subFunctionContext, &stack[sizeof(stack)], stack, &subFunction);

    // swap to new context, parentContext will point to the current context,
    //  resuming execution right after the call to swap_context
    swap_context_on_top(&subFunctionContext, nullptr, [](Context* parent, void*){parentContext = *parent;});
    EXPECT_TRUE(subFunctionStep1);
    EXPECT_FALSE(subFunctionStep2);

    // swap to context inside subFunction
    swap_context_on_top(&subfunctionInnerContext, nullptr, [](Context* parent, void*){parentContext = *parent;});

    EXPECT_TRUE(subFunctionStep1);
    EXPECT_TRUE(subFunctionStep2);
}

std::atomic<bool> onTop1 = false;
std::atomic<bool> onTop2 = false;
std::atomic<bool> calledSubFunctionWithOnTop1 = false;
std::atomic<bool> calledSubFunctionWithOnTop2 = false;

static void subFunctionWithOnTop() {
    EXPECT_TRUE(onTop1); // on top function should have modified this
    EXPECT_FALSE(onTop2);
    EXPECT_FALSE(calledSubFunctionWithOnTop1);
    calledSubFunctionWithOnTop1 = true;

    // yield execution to parent
    EXPECT_TRUE(onTop1);
    EXPECT_FALSE(onTop2);
    swap_context_on_top(&parentContext, nullptr, [](Context* parent, void*){subfunctionInnerContext = *parent;});
    EXPECT_TRUE(onTop1);
    EXPECT_TRUE(onTop2); // 2nd on top function should have modified this

    EXPECT_FALSE(calledSubFunctionWithOnTop2);
    calledSubFunctionWithOnTop2 = true;

    swap_context_on_top(&parentContext, nullptr, [](Context*,void*){});
    FAIL(); // will never execute
}

TEST(SwapContext, ExecuteOnTop) {
    // setup new context
    alignas(16) char stack[4096*5] = {0};
    Context subFunctionContext;
    make_context(&subFunctionContext, &stack[sizeof(stack)], stack, &subFunctionWithOnTop);

    EXPECT_FALSE(calledSubFunctionWithOnTop1);
    EXPECT_FALSE(calledSubFunctionWithOnTop2);
    EXPECT_FALSE(onTop1);
    EXPECT_FALSE(onTop2);
    // swap to new context, parentContext will point to the current context,
    //  resuming execution right after the call to swap_context
    swap_context_on_top(&subFunctionContext, nullptr, [](Context* parent, void*) {
        parentContext = *parent;
        EXPECT_FALSE(calledSubFunctionWithOnTop1);
        EXPECT_FALSE(onTop1);
        onTop1 = true;
        // when this lambda returns, go inside subFunctionWithOnTop
    });
    EXPECT_TRUE(calledSubFunctionWithOnTop1);
    EXPECT_TRUE(onTop1);
    EXPECT_FALSE(calledSubFunctionWithOnTop2);
    EXPECT_FALSE(onTop2);

    // swap to context inside subFunction
    swap_context_on_top(&subfunctionInnerContext, nullptr, [](Context* parent, void*) {
        parentContext = *parent;
        EXPECT_FALSE(calledSubFunctionWithOnTop2);
        EXPECT_FALSE(onTop2);
        onTop2 = true;
        // when this lambda returns, go inside subFunctionWithOnTop
    });
    EXPECT_TRUE(calledSubFunctionWithOnTop1);
    EXPECT_TRUE(onTop1);
    EXPECT_TRUE(calledSubFunctionWithOnTop2);
    EXPECT_TRUE(onTop2);
}
