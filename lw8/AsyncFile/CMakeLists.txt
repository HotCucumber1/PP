add_executable(AsyncFile main.cpp)

target_link_libraries(AsyncFile PRIVATE uring)
