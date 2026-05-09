add_executable(TestAtomicMax test/AtomicMaxTest.cpp)
add_executable(BenchmarkAtomicMax test/AtomicMaxBenchmark.cpp)

target_link_libraries(TestAtomicMax PRIVATE Catch2::Catch2WithMain)
target_link_libraries(BenchmarkAtomicMax PRIVATE Catch2::Catch2WithMain)
