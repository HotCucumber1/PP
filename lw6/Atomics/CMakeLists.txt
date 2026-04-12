add_executable(
        Atomics
        Image.cpp
        Histogram.cpp
        main.cpp)

target_link_libraries(Atomics PRIVATE Catch2::Catch2WithMain)
target_link_libraries(Atomics PRIVATE TBB::tbb)
