//
// Created by jglrxavpok on 23/09/2023.
//

#include <gtest/gtest.h>
#include <span>
#include <cider/Fiber.h>

struct Data {
    int number = 42;
    std::string string = "Hello world!";

    float pi = 3;
};

static void fiberProc(Cider::FiberHandle& fiber, void* userData) {
    Data* pData = (Data*)userData;
    pData->number = 50;
    pData->string = "Hi!";
    pData->pi = 3.14f;

    fiber.yield();

    pData->number = 100;
    pData->string = "Hi !!!";
    pData->pi = 3.14159f;
}

TEST(SimpleFiber, FunctionPointers) {
    alignas(16) char stack[4096*2] = {0};
    Data myData;
    Cider::Fiber fiber { fiberProc, &myData, std::span(stack) };

    EXPECT_EQ(myData.number, 42);
    EXPECT_EQ(myData.string, "Hello world!");
    EXPECT_FLOAT_EQ(myData.pi, 3.0f);

    fiber.switchTo();

    EXPECT_EQ(myData.number, 50);
    EXPECT_EQ(myData.string, "Hi!");
    EXPECT_FLOAT_EQ(myData.pi, 3.14f);

    fiber.switchTo();

    EXPECT_EQ(myData.number, 100);
    EXPECT_EQ(myData.string, "Hi !!!");
    EXPECT_FLOAT_EQ(myData.pi, 3.14159f);

    EXPECT_EXIT(fiber.switchTo(), ::testing::ExitedWithCode(255), "Reached bottom of fiber callstack"); // fiber has reached bottom of callstack, returning is not allowed at this point
}

TEST(SimpleFiber, StdFunction) {
    alignas(16) char stack[4096*5];
    Data myData;

    auto myFunc = [&myData](Cider::FiberHandle& fiber) {
        fiberProc(fiber, &myData);
    };
    Cider::Fiber fiber { myFunc, std::span(stack) };

    EXPECT_EQ(myData.number, 42);
    EXPECT_EQ(myData.string, "Hello world!");
    EXPECT_FLOAT_EQ(myData.pi, 3.0f);

    fiber.switchTo();

    EXPECT_EQ(myData.number, 50);
    EXPECT_EQ(myData.string, "Hi!");
    EXPECT_FLOAT_EQ(myData.pi, 3.14f);

    fiber.switchTo();

    EXPECT_EQ(myData.number, 100);
    EXPECT_EQ(myData.string, "Hi !!!");
    EXPECT_FLOAT_EQ(myData.pi, 3.14159f);

    EXPECT_EXIT(fiber.switchTo(), ::testing::ExitedWithCode(255), "Reached bottom of fiber callstack"); // fiber has reached bottom of callstack, returning is not allowed at this point
}

TEST(SimpleFiber, FiberReuse) {
    alignas(16) char stack[1024*8];
    Data myData;

    auto root = [&myData](Cider::FiberHandle& fiber) {
        while(true) {
            fiber.yield();
        }
    };
    Cider::Fiber fiber { root, std::span(stack) };

    EXPECT_EQ(myData.number, 42);
    // task 1
    fiber.switchToWithOnTop([&](Cider::FiberHandle& fiber) {
        myData.number = 3;
        fiber.yield();
        myData.number = 5;
    });
    EXPECT_EQ(myData.number, 3);
    fiber.switchTo();
    EXPECT_EQ(myData.number, 5);
    // task 2
    fiber.switchToWithOnTop([&](Cider::FiberHandle& fiber) {
        myData.number = 4;
        fiber.yield();
        myData.number = 6;
    });
    EXPECT_EQ(myData.number, 4);
    fiber.switchTo();
    EXPECT_EQ(myData.number, 6);

    EXPECT_NO_FATAL_FAILURE(fiber.switchTo()); // should not crash
}