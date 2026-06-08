# Concurrency benötigt explizites pthread-Linking auf Linux
add_example(11_concurrency main.cpp)
target_link_libraries(11_concurrency PRIVATE Threads::Threads)
