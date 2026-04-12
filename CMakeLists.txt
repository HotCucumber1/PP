cmake_minimum_required(VERSION 3.29)
project(PP)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG v3.1.1
)
FetchContent_MakeAvailable(cxxopts)
include_directories(${cxxopts_SOURCE_DIR})

find_package(TBB REQUIRED)

add_subdirectory(lw1/MakeArchive)
add_subdirectory(lw1/ExtractArchive)

add_subdirectory(lw2/Life)
add_subdirectory(lw2/Gauss)

add_subdirectory(lw3/Bank)
add_subdirectory(lw3/Warehouse)

add_subdirectory(lw4/MusicPlayer)

add_subdirectory(lw5/TV)

add_subdirectory(lw6/Atomics)
