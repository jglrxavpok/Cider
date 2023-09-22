Cider - Standalone fibers
====================

Cider aims to implement [Fibers](https://en.wikipedia.org/wiki/Fiber_(computer_science)) with a simple interface and 
fast context switching.

Partially based on https://graphitemaster.github.io/fibers/ (warning, it has 2 errors: RSP is not read properly in get_context, 
and first integer argument is RCX on Windows, not RDX)

Windows only for now.

# Usage
TODO (fibers not fully implemented, only context switching)

# Context-switch primitives
```cpp
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
```

# Code structure
- `include/context.h`: Basic primitives for userland context switching
- `include/internal`: platform specific structures
- TODO: fiber stuff