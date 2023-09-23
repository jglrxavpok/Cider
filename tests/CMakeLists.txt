FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
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

        # Fiber synchronisation primitives
        # TODO
)
target_include_directories(Tests PUBLIC "../include")
target_link_libraries(
        Tests
        PUBLIC gtest_main cider
)

include(GoogleTest)
gtest_discover_tests(Tests)
enable_testing()