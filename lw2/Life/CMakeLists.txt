cmake_policy(SET CMP0167 NEW)

find_package(Boost REQUIRED COMPONENTS thread)

add_executable(Life LifeGame/LifeGame.cpp main.cpp)

target_include_directories(Life PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(Life PRIVATE Boost::thread Threads::Threads)
