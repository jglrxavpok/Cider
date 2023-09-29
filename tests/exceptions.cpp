//
// Created by jglrxavpok on 22/09/2023.
//

#include <gtest/gtest.h>
#include <span>
#include <cider/Fiber.h>
#include <cider/GrowingStack.h>


TEST(Exceptions, CatchInner) {
    char stack[4096*16]{0};
    bool caught = false;
    auto proc = [&](Cider::FiberHandle& fiber) {
        try {
            fiber.yield();
            throw std::invalid_argument("Test");
        } catch(const std::invalid_argument&) {
            caught = true;
        };
    };
    Cider::Fiber fiber { proc, std::span(stack) };
    fiber.switchTo();
    EXPECT_FALSE(caught);
    fiber.switchTo();
    EXPECT_TRUE(caught);
}

TEST(Exceptions, CatchInsideMallocedStack) {
    char* stack = (char*)malloc(4096*16);
    memset(stack, 0, 4096*16);
    bool caught = false;
    auto proc = [&](Cider::FiberHandle& fiber) {
        try {
            fiber.yield();
            throw std::invalid_argument("Test");
        } catch(const std::invalid_argument&) {
            caught = true;
        };
    };
    Cider::Fiber fiber { proc, std::span(stack, 4096*16) };
    fiber.switchTo();
    EXPECT_FALSE(caught);
    fiber.switchTo();
    EXPECT_TRUE(caught);
    free(stack);
}

TEST(Exceptions, CatchInsideAllocaedStack) {
    char* stack = (char*) _malloca(4096*16);
    memset(stack, 0, 4096*16);
    bool caught = false;
    auto proc = [&](Cider::FiberHandle& fiber) {
        try {
            fiber.yield();
            throw std::invalid_argument("Test");
        } catch(const std::invalid_argument&) {
            caught = true;
        };
    };
    Cider::Fiber fiber { proc, std::span(stack, 4096*16) };
    fiber.switchTo();
    EXPECT_FALSE(caught);
    fiber.switchTo();
    EXPECT_TRUE(caught);
    _freea(stack);
}

TEST(Exceptions, CatchInsideGrowingStack) {
    Cider::GrowingStack stack{4096*16};
    bool caught = false;
    auto proc = [&](Cider::FiberHandle& fiber) {
        try {
            fiber.yield();
            throw std::invalid_argument("Test");
        } catch(const std::invalid_argument&) {
            caught = true;
        };
    };
    Cider::Fiber fiber { proc, stack.asSpan() };
    fiber.switchTo();
    EXPECT_FALSE(caught);
    fiber.switchTo();
    EXPECT_TRUE(caught);
}