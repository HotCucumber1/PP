cmake_policy(SET CMP0167 NEW)

find_package(Boost REQUIRED COMPONENTS system thread)

add_executable(TestLFThreadPool LFThreadPool.cpp LBThreadPool.cpp test/ThreadPoolTest.cpp)
add_executable(BenchmarkLFThreadPool LFThreadPool.cpp LBThreadPool.cpp main.cpp)

target_include_directories(TestLFThreadPool PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(TestLFThreadPool PRIVATE Boost::thread Threads::Threads)
target_link_libraries(TestLFThreadPool PRIVATE Catch2::Catch2WithMain)

target_include_directories(BenchmarkLFThreadPool PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(BenchmarkLFThreadPool PRIVATE Boost::thread Threads::Threads)
