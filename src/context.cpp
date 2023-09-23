//
// Created by jglrxavpok on 23/09/2023.
//

/**
 * Implementation of C++ helpers, not required if you just need
 */

#include <cider/context.hpp>
#include <memory>

void swapContextOnTop(Context* pCurrent, Context* pToSwitchTo, std::function<void()> onTop) {
    using OnTopFunc = std::function<void()>;
    auto* copy = new OnTopFunc(std::move(onTop));

    // TODO: support for exceptions
    swap_context_on_top(pCurrent, pToSwitchTo, copy, [](void* pUserData) {
        auto pFunc = std::unique_ptr<OnTopFunc>((OnTopFunc*)pUserData); // ensure function copy is deleted at some point
        (*pFunc)();
    });
}