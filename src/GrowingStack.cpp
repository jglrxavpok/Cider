//
// Created by jglrxavpok on 26/09/2023.
//

#include "cider/GrowingStack.h"
#include <stdexcept>

#ifdef _WIN64
#include <windows.h>

struct Impl {
    void* allocAddress = nullptr;
    SIZE_T requestedSize = 0;

    explicit Impl(std::size_t maxSize) : requestedSize(maxSize) {
        allocAddress = VirtualAlloc(nullptr, maxSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if(!allocAddress) {
            throw std::invalid_argument("Could not allocate virtual stack");
        }
    }

    ~Impl() {
        VirtualFree(allocAddress, 0, MEM_RELEASE);
    }

    void withStack(const std::function<void()>& f) {
        f();
    }

    std::span<char> asSpan() {
        return std::span{ (char*)allocAddress, requestedSize };
    }
};
#else
#error No implementation for this platform.
#endif

namespace Cider {

    Cider::GrowingStack::GrowingStack(std::size_t maxSize) {
        pImpl = new Impl(maxSize);
    }

    GrowingStack::~GrowingStack() {
        delete (Impl*)pImpl;
    }

    void GrowingStack::withStack(const std::function<void()>& f) {
        ((Impl*)pImpl)->withStack(f);
    }

    std::span<char> GrowingStack::asSpan() {
        return ((Impl*)pImpl)->asSpan();
    }
}