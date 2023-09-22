//
// Created by jglrxavpok on 20/09/2023.
//

#include <gtest/gtest.h>

#include <cider/context.h>

TEST(BasicContextSwitch, JumpBack) {
    std::atomic<int> passes = 0;
    std::atomic<int> passes2 = 0;

    Context context{0};
    get_context(&context);

    passes2++;
    if(passes == 0) {
        int t = passes;
        passes = t+1;
        set_context(&context); // will jump to line 14
    }

    EXPECT_EQ(passes2, 2);
}

static Context* goBackTo = nullptr;

static void someOtherFunction() {
    printf("Calling other function\n");
    set_context(goBackTo);
}

TEST(BasicContextSwitch, CallOtherFunction) {
    alignas(16) char otherFunctionStack[4096];

    std::atomic<bool> alreadyCalled = false;
    Context newContext = {0};
    newContext.rsp = (Register)(otherFunctionStack + sizeof(otherFunctionStack));
    newContext.rip = (Register)someOtherFunction;

    std::atomic<int> r = 0;

    // I need *some* address to go back to. Some come back here
    Context beforeCall;
    get_context(&beforeCall);

    // setup the return address
    goBackTo = &beforeCall;

    if(!alreadyCalled) {
        alreadyCalled = true;
        r++;
        set_context(&newContext);
    }

    printf("%d\n", r.load());
    EXPECT_TRUE(alreadyCalled);
}


static Context parentContext = {0};
static Context yieldPoint = {0};

static void yieldingFunction() {
    printf("Before yield\n");
    // yield
    {
        std::atomic<bool> yield = false;
        get_context(&yieldPoint);
        if(!yield) {
            yield = true;
            set_context(&parentContext);
        }
    }
    printf("After yield\n");
    set_context(&parentContext);
}

TEST(BasicContextSwitch, ProtoYield) {
    std::atomic<bool> startedCall = false;
    get_context(&parentContext);

    if(!startedCall) {
        startedCall = true;
        yieldingFunction();
    } else {
        printf("YieldTest: while function is yielding\n");
        std::atomic<bool> resumed = false;
        get_context(&parentContext);

        if(!resumed) {
            resumed = true;
            set_context(&yieldPoint);
        }
    }

    printf("finished!\n");
}