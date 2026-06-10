cmake_minimum_required(VERSION 3.29)
project(PP)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(OpenCL_LIBRARY "/usr/lib/x86_64-linux-gnu/libOpenCL.so.1")
set(OpenCL_INCLUDE_DIR "/usr/include")

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
find_package(OpenCL REQUIRED)

add_subdirectory(lw1/MakeArchive)
add_subdirectory(lw1/ExtractArchive)

add_subdirectory(lw2/Life)
add_subdirectory(lw2/Gauss)

add_subdirectory(lw3/Bank)
add_subdirectory(lw3/Warehouse)

add_subdirectory(lw4/MusicPlayer)

add_subdirectory(lw5/TV)

add_subdirectory(lw6/Atomics)
add_subdirectory(lw6/TicketOffice)
add_subdirectory(lw6/StopToken)

add_subdirectory(lw7/AtomicMax)
add_subdirectory(lw7/ThreadPool_LF)

add_subdirectory(lw8/MyTask)
add_subdirectory(lw8/BookGenerator)
add_subdirectory(lw8/MyAwaiter)
add_subdirectory(lw8/AsyncFile)

add_subdirectory(lw9/BitonicSort)
