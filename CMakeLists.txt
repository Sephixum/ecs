cmake_minimum_required(VERSION 3.25)

project(ecs
    VERSION 0.1.0
    DESCRIPTION "Archetype-based Entity Component System"
    LANGUAGES CXX
)

# ── Global settings ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # for clangd / tooling

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type set — defaulting to Debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif()

# ── Compiler warnings ──────────────────────────────────────────────────────────
add_library(ecs_warnings INTERFACE)
target_compile_options(ecs_warnings INTERFACE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
        -Wall -Wextra -Wpedantic
        -Wshadow -Wnon-virtual-dtor
        -Wcast-align -Woverloaded-virtual
        -Wnull-dereference -Wformat=2
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /permissive-
    >
)

# ── Sanitizers (Debug only) ────────────────────────────────────────────────────
add_library(ecs_sanitizers INTERFACE)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(ecs_sanitizers INTERFACE
        $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        >
    )
    target_link_options(ecs_sanitizers INTERFACE
        $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
            -fsanitize=address,undefined
        >
    )
endif()

# ── Library ────────────────────────────────────────────────────────────────────
add_library(ecs STATIC
    src/archetype.cpp
    src/column.cpp
    src/entity.cpp
    src/entity_handle.cpp
    src/signature.cpp
    src/world.cpp
)

target_include_directories(ecs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(ecs
    PUBLIC  ecs_warnings
    PRIVATE ecs_sanitizers
)

target_compile_options(ecs PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang,GNU>,$<CONFIG:Release>>:
        -O3 -march=native
    >
)

# ── Tests ──────────────────────────────────────────────────────────────────────
option(ECS_BUILD_TESTS "Build tests" ON)
if(ECS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
