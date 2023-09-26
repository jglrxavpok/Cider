Cider - Standalone fibers
====================

Cider aims to implement [Fibers](https://en.wikipedia.org/wiki/Fiber_(computer_science)) with a simple interface and 
fast context switching.

Partially based on https://graphitemaster.github.io/fibers/ (warning, it has 2 errors: RSP is not read properly in get_context, 
and first integer argument is RCX on Windows, not RDX)

Windows only for now. Relies on the STL, but it should be easy to replace types with your own.

# Usage
## Creating a fiber
Creating a fiber does not immediately start it.
Custom data can be provided to the fiber via a void*, but it is up to the user to free up ressources.

The std::function version of the constructor can help with lifetime management, as data can be added to the capture storage of the std::function.
```cpp
// using FiberProc = void(*)(Cider::FiberHandle& fiber, void* userData);
// using StdFunctionProc = std::function<void(Cider::FiberHandle&)>;
// Fiber(FiberProc proc, void* userData, std::span<char> stack);
// Fiber(StdFunctionProc proc, std::span<char> stack);

struct Data {
    int number = 42;
    std::string string = "Hello world!";

    float pi = 3;
};

alignas(16) char stack[4096] = {0}; // stack must be aligned on 16 bytes (at least on Windows), but the memory can come from anywhere
Data myData; // you are responsible for user data lifetime
auto myFunc = [&myData](Cider::FiberHandle& fiber) {
    // do stuff
};
Cider::Fiber fiber { myFunc, std::span(stack) };
```

## Starting/Resuming a fiber
```cpp
Cider::Fiber f /* = something*/;
f.switchTo(); // Starts running the code inside this fiber. When fiber finishes or yields, execution resumes from here
```
Warning! Calling `switchTo` on a fiber which has finished execution will crash the program! You will end up in FiberBottomOfCallstack (see Fiber.cpp)

## Yielding inside a fiber
You can yield the current fiber via its `FiberHandle`:
```cpp
void myFiberCode(Cider::FiberHandle& fiber) {
    // do stuff
    fiber.yield();
    // do other stuff
}
```
The `FiberHandle` object can be passed to sub-functions to yield. There is no way to obtain the current fiber without a FiberHandle.
This is to avoid surprise yields in code that looks synchronous.

## Fiber local storage
Because the same fiber can run on different threads over time, thread_local is not useable for fiber local storage.
Fiber local storage can be used to replace thread_local:
```cpp
int someFunction(Cider::FiberHandle& fiber, int arg0, char arg2, std::string someText) {
    return ((int*)fiber.localStorage.data())[0];
}
```
`FiberHandle::localStorage` is a `std::array<char, SIZE>`. If more memory is needed, consider saving a pointer inside. 
You can also store a pointer to something inside the stack inside your fiber proc.

# Context-switch primitives
`context.h` contains the declarations of switch primitives, implemented in Assembly in `src/context_masm.asm`. No other 
files are required if you only need these primitives.

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
extern "C" void swap_context_on_top(Context* pCurrent, Context* pToSwitchTo, void* userData, void (*func)(void*));
```

`context.hpp` (implementation `context.cpp`) provides a few helpers for C++ too, most notably support for std::function and
their captures.


# Code structure
Cider has 3 layers, with each layer building on top of the other:
1. Userland context switches:
    Fast context switches, without going through the OS.
    - `include/context.h`: Basic primitives for userland context switching
    - `include/context.hpp`: Imports `context.h`, helpers for smoother C++ experience.

2. Fibers:
    Write asynchronous code as if it was synchronous.
    - `include/Fiber.h`: `struct FiberHandle` and `class Fiber`. Allows usage of fibers

3. Fiber-aware synchronisation primitives:
    Locks, condition variables, latches for fibers.
    - `include\Mutex.h` & `include\WaitQueue.h`

4. Fiber scheduling
    Cider exposes the `Scheduler` base class that can be extended to provide your own scheduler. By default, fibers start 
    with a reference to the `GreedyScheduler` which immediately runs fibers when they are scheduled.