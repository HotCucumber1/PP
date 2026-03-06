add_executable(
        Bank
        Bank.cpp
        SpringfieldLife.cpp
        main.cpp)
add_executable(
        TestBank
        Bank.cpp
        SpringfieldLife.cpp
        test/TestBank.cpp)

target_link_libraries(TestBank PRIVATE Catch2::Catch2WithMain)