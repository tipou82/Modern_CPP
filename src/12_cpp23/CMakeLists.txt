# C++23 features require g++-12 or newer.
# The root CMakeLists.txt selects g++-12 automatically when available,
# so no per-target compiler override is needed here.
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12")
    message(STATUS "C++23: Compiler OK (${CMAKE_CXX_COMPILER}, v${CMAKE_CXX_COMPILER_VERSION})")
    add_executable(12_cpp23 main.cpp)
    set_target_properties(12_cpp23 PROPERTIES
        CXX_STANDARD          23
        CXX_STANDARD_REQUIRED ON)
else()
    message(STATUS "C++23: Compiler zu alt (${CMAKE_CXX_COMPILER_VERSION}) – 12_cpp23 wird übersprungen.")
    message(STATUS "       Installieren: sudo apt install g++-12")
endif()
