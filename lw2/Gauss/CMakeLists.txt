FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
)
FetchContent_MakeAvailable(stb)
include_directories(${stb_SOURCE_DIR})

add_executable(Gauss GaussBlur.cpp main.cpp)
