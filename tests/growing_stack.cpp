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

#ifdef _WIN64
#include <windows.h>

static int filter(DWORD exceptionCode) {
    return exceptionCode  == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

// try and __try cannot be in the same function (MSVC limitation)
static bool triggerAccessViolation(char* accessInGuardPage) {
    bool triggeredAccessViolation = false;
    __try {
        printf("Invalid: %s", accessInGuardPage); // reading inside guard page should trigger an access violation
    } __except(filter(GetExceptionCode())) {
        triggeredAccessViolation = true;
        printf("ACCESS VIOLATION!");
    }
    printf("\n");
    return triggeredAccessViolation;
}

TEST(GrowingStack, DetectStackOverflow) {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    DWORD pageSize = systemInfo.dwPageSize;

    Cider::GrowingStack stack{ pageSize };

    stack.withStack([&]() {
        char* accessInValidPage = stack.asSpan().data();
        char* accessInGuardPage = stack.asSpan().data() - pageSize;
        strcpy(accessInValidPage, "Hello world!");
        EXPECT_NO_THROW(printf("Valid: %s\n", accessInValidPage));
        EXPECT_TRUE(triggerAccessViolation(accessInGuardPage));
    });
}
#endif