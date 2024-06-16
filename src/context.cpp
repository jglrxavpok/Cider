//
// Created by jglrxavpok on 23/09/2023.
//

/**
 * Implementation of C++ helpers, not required if you just need
 */

#include <cider/context.hpp>
#include <memory>

void swapContextOnTop(Context* pToSwitchTo, std::function<void(Context* parentContext)> onTop) {
    using OnTopFunc = std::function<void(Context* parentContext)>;
    auto* copy = new OnTopFunc(std::move(onTop));

    // TODO: support for exceptions
    swap_context_on_top(pToSwitchTo, copy, [](Context* parentContext, void* pUserData) {
        auto pFunc = std::unique_ptr<OnTopFunc>((OnTopFunc*)pUserData); // ensure function copy is deleted at some point
         (*pFunc)(parentContext);
    });
}
