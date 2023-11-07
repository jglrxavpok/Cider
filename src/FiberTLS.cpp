//
// Created by jglrxavpok on 07/11/2023.
//

#include <cider/Fiber.h>

namespace Cider {
    thread_local Fiber* pCurrentFiberTLS = nullptr;

    Fiber*& Fiber::getCurrentFiberTLS() {
        return pCurrentFiberTLS;
    }
}