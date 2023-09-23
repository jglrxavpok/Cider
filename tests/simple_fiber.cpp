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
    alignas(16) char stack[4096] = {0};
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

    EXPECT_EXIT(fiber.switchTo(), ::testing::ExitedWithCode(-1), "Reached bottom of fiber callstack"); // fiber has reached bottom of callstack, returning is not allowed at this point
}

TEST(SimpleFiber, StdFunction) {
    alignas(16) char stack[4096];
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

    EXPECT_EXIT(fiber.switchTo(), ::testing::ExitedWithCode(-1), "Reached bottom of fiber callstack"); // fiber has reached bottom of callstack, returning is not allowed at this point
}