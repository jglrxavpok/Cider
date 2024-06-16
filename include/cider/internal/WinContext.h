//
// Created by jglrxavpok on 20/09/2023.
//

#pragma once

#include <emmintrin.h>
#include <cstdint>

using Register = std::uintptr_t;
using Address = std::uint64_t;

// https://learn.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-170
struct WinContext {
    // Instruction pointer
    Register rip;
    // Stack pointer
    Register rsp;

    // Non volatile registers
    Register r12;
    Register r13;
    Register r14;
    Register r15;
    Register rdi;
    Register rsi;
    Register rbx;
    Register rbp;

    // Non volatile vector registers
    __m128i xmm6;
    __m128i xmm7;
    __m128i xmm8;
    __m128i xmm9;
    __m128i xmm10;
    __m128i xmm11;
    __m128i xmm12;
    __m128i xmm13;
    __m128i xmm14;
    __m128i xmm15;

    // TIB information
    Address stackLowAddress;
    Address stackHighAddress;
    Address deallocationStack;
    Register guaranteedStackBytes;

    // need MMX control word
    // need MMX(?) Status control word
    // need x87 control word
};