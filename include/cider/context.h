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

extern "C" {
 /**
  * Store the current context inside a given WinContext* (stored in pCurrentContext)
  */
 void __cdecl get_context(Context* pCurrentContext);

 /**
  * Restores the context from a WinContext* (stored in pContextToSwitchTo). Can be used to return to a context made
  * with get_context, or a brand new context
  */
 void __cdecl set_context(Context* pContextToSwitchTo);

 /**
  * Resumings the given context by calling the given function in 'func'
  * When returning, that function will return to the context inside 'pToSwitchTo', ie 'func' acts as if the context inside 'pToSwitchTo' called it
  */
 void __cdecl swap_context_on_top(Context* pToSwitchTo, void* userData, void (*func)(Context* parentContext, void* userData));

}