//
// Created by jglrxavpok on 20/09/2023.
//

#pragma once

#ifdef _WIN64
#include <cider/internal/WinContext.h>
using Context = WinContext;
#else
#error Platform not supported, good luck.
#endif

/**
 * Store the current context inside a given WinContext* (stored in pCurrentContext)
 */
extern "C" void get_context(Context* pCurrentContext);

/**
 * Restores the context from a WinContext* (stored in pContextToSwitchTo). Can be used to return to a context made
 * with get_context, or a brand new context
 */
extern "C" void set_context(Context* pContextToSwitchTo);

/**
 * Swaps the current context (will be stored in pCurrent) with another context (read from pToSwitchTo).
 * When switching to pCurrent, execution will resume after this call
 */
extern "C" void swap_context(Context* pCurrent, Context* pToSwitchTo);

/**
 * Slightly different version of swap_context,
 *  resuming the context by calling the given function in 'func'
 * When returning, that function will return to the context inside 'pToSwitchTo'
 */
extern "C" void swap_context_on_top(Context* pCurrent, Context* pToSwitchTo, void (*func)());