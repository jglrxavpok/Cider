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
    SIZE_T totalSize = 0;

    explicit Impl(std::size_t maxSize) : requestedSize(maxSize) {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        DWORD pageSize = systemInfo.dwPageSize;

        totalSize = maxSize + pageSize;
        allocAddress = VirtualAlloc(nullptr, totalSize, MEM_RESERVE, PAGE_NOACCESS);

        if(!allocAddress) {
            throw std::invalid_argument("Could not allocate virtual stack");
        }

        // allocate a no-access page at the top of the stack to catch stack overflows
        void* protectionPage = VirtualAlloc(allocAddress, pageSize, MEM_COMMIT, PAGE_NOACCESS);
        if(!protectionPage) {
            throw std::invalid_argument("Could not allocate protection page");
        }

        void* committed = VirtualAlloc((void*)((std::size_t)allocAddress + pageSize), requestedSize, MEM_COMMIT, PAGE_READWRITE);
        if(!committed) {
            throw std::invalid_argument("Could not commit stack memory");
        }
    }

    ~Impl() {
        VirtualFree(allocAddress, 0, MEM_RELEASE);
    }

    void withStack(const std::function<void()>& f) {
        f();
    }

    std::span<char> asSpan() {
        return std::span{ (char*)allocAddress, totalSize };
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