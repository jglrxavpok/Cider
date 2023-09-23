//
// Created by jglrxavpok on 23/09/2023.
//

#include <gtest/gtest.h>
#include <span>
#include <cider/Fiber.h>

struct MyLocalStorage {
    std::string text;
};

MyLocalStorage& getLocalStorage(Cider::FiberHandle& fiber) {
    return *((MyLocalStorage*)fiber.localStorage.data());
}

TEST(FiberLocalStorage, Test0) {
    alignas(16) char stack1[4096];
    alignas(16) char stack2[4096];

    auto fiberCode1 = [](Cider::FiberHandle& fiber) {
        static_assert(sizeof(MyLocalStorage) <= sizeof(Cider::FiberHandle::localStorage));

        getLocalStorage(fiber).text = "Hello local storage!";
        fiber.yield();
        EXPECT_EQ(getLocalStorage(fiber).text, "Hello local storage!");
    };
    auto fiberCode2 = [](Cider::FiberHandle& fiber) {
        static_assert(sizeof(MyLocalStorage) <= sizeof(Cider::FiberHandle::localStorage));

        getLocalStorage(fiber).text = "Hello local storage 2!";
        fiber.yield();
        EXPECT_EQ(getLocalStorage(fiber).text, "Hello local storage 2!");
    };

    Cider::Fiber fiber1 { fiberCode1, std::span(stack1) };
    fiber1.switchTo();
    // fiber1 local storage has "Hello local storage!"

    Cider::Fiber fiber2 { fiberCode2, std::span(stack2) };
    fiber2.switchTo();
    // fiber1 local storage has "Hello local storage!"
    // fiber2 local storage has "Hello local storage 2!"

    // run EXPECT lines to verify the contents is correct
    fiber1.switchTo();
    fiber2.switchTo();
}