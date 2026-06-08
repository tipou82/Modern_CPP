cmake_minimum_required(VERSION 3.20)

# Prefer g++-12 or newer so C++23 (Thema 12) compiles without a per-target hack.
# Must be set before project() – later assignments are ignored by CMake.
if(NOT DEFINED CMAKE_CXX_COMPILER)
    find_program(_GXX12 NAMES g++-12 g++-13 g++-14)
    if(_GXX12)
        set(CMAKE_CXX_COMPILER "${_GXX12}" CACHE FILEPATH "C++ compiler" FORCE)
    endif()
endif()

project(Modern_CPP CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# Threads-Bibliothek für Concurrency (pthread auf Linux)
find_package(Threads REQUIRED)

# Jede .cpp-Datei wird zu einem eigenen ausführbaren Programm
function(add_example name source)
    add_executable(${name} ${source})
endfunction()

add_subdirectory(src/01_basics)
add_subdirectory(src/02_smart_pointers)
add_subdirectory(src/03_move_semantik)
add_subdirectory(src/04_templates)
add_subdirectory(src/05_lambdas)
add_subdirectory(src/06_optional_variant)
add_subdirectory(src/07_enum_class)
add_subdirectory(src/08_stl_containers)
add_subdirectory(src/09_type_traits)
add_subdirectory(src/10_chrono)
add_subdirectory(src/11_concurrency)
add_subdirectory(src/12_cpp23)
