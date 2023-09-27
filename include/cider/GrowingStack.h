//
// Created by jglrxavpok on 26/09/2023.
//

#pragma once
#include <span>
#include <cstdint>
#include <functional>

namespace Cider {
    /**
     * Stack that uses virtual allocations to grow when needed
     */
    class GrowingStack {
    public:
        /**
         * Initializes a GrowingStack.
         * @param maxSize maximum size of the stack
         */
        explicit GrowingStack(std::size_t maxSize);
        ~GrowingStack();

        /**
         * Runs the given function, with machinery necessary to grow the stack as needed.
         * @param f function to run while using this stack
         */
        void withStack(const std::function<void()>& f);

        /**
         * Span representing the entire address space available to this stack
         * @return
         */
        std::span<char> asSpan();

    private:
        void* pImpl = nullptr;
    };
}