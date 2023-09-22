//
// Created by jglrxavpok on 22/09/2023.
//

#include <gtest/gtest.h>

#include <cider/context.h>

std::atomic<bool> subFunctionStep1 = false;
std::atomic<bool> subFunctionStep2 = false;
static Context parentContext = {0};
static Context subfunctionInnerContext = {0};

static void subFunction() {
    EXPECT_FALSE(subFunctionStep1);
    subFunctionStep1 = true;

    // yield execution to parent
    swap_context(&subfunctionInnerContext, &parentContext);

    EXPECT_FALSE(subFunctionStep2);
    subFunctionStep2 = true;

    set_context(&parentContext);
    FAIL(); // will never execute
}

TEST(SwapContext, SwapYieldAndContinue) {
    EXPECT_FALSE(subFunctionStep1);
    EXPECT_FALSE(subFunctionStep2);

    // setup new context
    alignas(16) char stack[4096] = {0};
    Context subFunctionContext = {0};
    subFunctionContext.rip = (std::uint64_t)&subFunction;
    subFunctionContext.rsp = (std::uint64_t)stack;

    // swap to new context, parentContext will point to the current context,
    //  resuming execution right after the call to swap_context
    swap_context(&parentContext, &subFunctionContext);
    EXPECT_TRUE(subFunctionStep1);
    EXPECT_FALSE(subFunctionStep2);

    // swap to context inside subFunction
    swap_context(&parentContext, &subfunctionInnerContext);

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
    swap_context(&subfunctionInnerContext, &parentContext);
    EXPECT_TRUE(onTop1);
    EXPECT_TRUE(onTop2); // 2nd on top function should have modified this

    EXPECT_FALSE(calledSubFunctionWithOnTop2);
    calledSubFunctionWithOnTop2 = true;

    set_context(&parentContext);
    FAIL(); // will never execute
}

TEST(SwapContext, ExecuteOnTop) {
    // setup new context
    alignas(16) char stack[4096] = {0};
    Context subFunctionContext = {0};
    subFunctionContext.rip = (std::uint64_t)&subFunctionWithOnTop;
    subFunctionContext.rsp = (std::uint64_t)stack;

    EXPECT_FALSE(calledSubFunctionWithOnTop1);
    EXPECT_FALSE(calledSubFunctionWithOnTop2);
    EXPECT_FALSE(onTop1);
    EXPECT_FALSE(onTop2);
    // swap to new context, parentContext will point to the current context,
    //  resuming execution right after the call to swap_context
    swap_context_on_top(&parentContext, &subFunctionContext, []() {
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
    swap_context_on_top(&parentContext, &subfunctionInnerContext, []() {
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
