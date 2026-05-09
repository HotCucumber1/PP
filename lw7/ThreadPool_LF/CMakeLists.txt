cmake_policy(SET CMP0167 NEW)

find_package(Boost REQUIRED COMPONENTS system thread)

add_executable(LFThreadPool LFThreadPool.cpp LBThreadPool.cpp test/ThreadPoolTest.cpp)

target_include_directories(LFThreadPool PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(LFThreadPool PRIVATE Boost::thread Threads::Threads)
