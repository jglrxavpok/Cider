//
// Created by jglrxavpok on 21/04/25.
//

#pragma once

#include <emmintrin.h>
#include <cstdint>

using Register = std::uintptr_t;
using Address = std::uint64_t;

// https://wiki.osdev.org/System_V_ABI
struct LinuxContext {
    Register rip;
    Register rsp;

    Register rbx;
    Register rbp;
    Register r12;
    Register r13;
    Register r14;
    Register r15;

    // Note: no x87 FPU state for now
};

static_assert(sizeof(LinuxContext) == 64); // don't forget to change context_linux_x64.S if LinuxContext changes