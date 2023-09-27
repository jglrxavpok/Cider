//
// Created by jglrxavpok on 22/09/2023.
//

#include <gtest/gtest.h>

#include <cider/Fiber.h>
#include <cider/GrowingStack.h>

static void allocateOnStack(int depth) {
    if(depth > 0) {
        char someData[2048]{0};
        for(int i = 0; i < sizeof(someData); i++) {
            someData[i] = (char)i;
        }
        allocateOnStack(depth-1);
        printf("%d\n", someData[2047]);
    }
}

TEST(GrowingStack, AllocateALot) {
    Cider::GrowingStack stack{ 64*1024*1024 };

    auto fiberProc = [&](Cider::FiberHandle& handle) {
        stack.withStack([&]() {
            allocateOnStack(50);
        });
    };

    Cider::Fiber fiber(fiberProc, stack.asSpan());
    fiber.switchTo();
}
