cmake_minimum_required(VERSION 3.24)
project(cider)

set(CMAKE_CXX_STANDARD 20)

# ASM support
# https://discourse.cmake.org/t/building-lib-file-from-asm-cmake-bug/1959
enable_language(ASM_MASM) # required for the context manipulation routines
set(CMAKE_ASM_MASM_CREATE_STATIC_LIBRARY "<CMAKE_AR> /OUT:<TARGET> <LINK_FLAGS> <OBJECTS>")

option(CIDER_ASAN "Enable ASAN support in Cider" OFF) # does not work very well for now
if(CIDER_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif(CIDER_ASAN)


include(FetchContent)
set(Cider-Sources
        src/context_masm.asm
        src/context.cpp
        src/Fiber.cpp
        src/GrowingStack.cpp
        src/Mutex.cpp
        src/WaitQueue.cpp

        src/internal/SpinLock.cpp
        src/scheduling/GreedyScheduler.cpp
)
add_library(cider ${Cider-Sources})
target_include_directories(cider PUBLIC include)

# Disable RTC for MSVC
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()


add_subdirectory(tests)
