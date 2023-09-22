Cider - Standalone fibers
====================

Cider aims to implement [Fibers](https://en.wikipedia.org/wiki/Fiber_(computer_science)) with a simple interface and 
fast context switching.

Partially based on https://graphitemaster.github.io/fibers/ (warning, it has 2 errors: RSP is not read properly in get_context, 
and first integer argument is RCX on Windows, not RDX)

Windows only for now.

# Code structure
- `include/context.h`: Basic primitives for userland context switching
- `include/internal`: platform specific structures
- TODO: fiber stuff