FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/1b96fa13f549387b7549cc89e1a785cf143a1a50.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(
        Tests
        # Context stuff
        basic_context_switch.cpp
        swap_context.cpp

        # Fibers
        simple_fiber.cpp
        fiber_local_storage.cpp
        growing_stack.cpp
        exceptions.cpp

        # Fiber synchronisation primitives
        fiber_mutex.cpp
)
target_include_directories(Tests PUBLIC "../include")
target_link_libraries(
        Tests
        PUBLIC gtest_main cider
)

include(GoogleTest)
gtest_discover_tests(Tests)
enable_testing()