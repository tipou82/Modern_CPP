# C++23 benötigt g++-12 oder neuer.
# Wird automatisch übersprungen wenn kein passender Compiler gefunden wird.
find_program(GXX12 NAMES g++-12 g++-13 g++-14)

if(GXX12)
    message(STATUS "C++23: Compiler gefunden: ${GXX12}")
    add_executable(12_cpp23 main.cpp)
    # Nur dieses Target bekommt C++23 – alle anderen bleiben bei C++20
    set_target_properties(12_cpp23 PROPERTIES
        CXX_STANDARD          23
        CXX_STANDARD_REQUIRED ON
        RULE_LAUNCH_COMPILE   "${GXX12}")
else()
    message(STATUS "C++23: Kein g++-12/13/14 gefunden – 12_cpp23 wird übersprungen.")
    message(STATUS "       Installieren: sudo apt install g++-12")
endif()
