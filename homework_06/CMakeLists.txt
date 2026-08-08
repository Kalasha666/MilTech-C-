cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 04. Його можна збирати як
# через root CMakeLists.txt, так і напряму з цієї директорії.
project(ugv_ballistics CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ballistics_lib src/ballistics.cpp)
target_include_directories(ballistics_lib PUBLIC include)
# Один executable для задачі з UGV ballistics.
add_executable(ugv_ballistics src/main.cpp)
target_link_libraries(ugv_ballistics PRIVATE ballistics_lib)

# Базові compiler warnings допомагають побачити підозрілі місця до runtime.
target_compile_options(ugv_ballistics PRIVATE -Wall -Wextra -pedantic)

add_executable(ballistics_tests tests/ballistics_tests.cpp)
target_link_libraries(ballistics_tests PRIVATE ballistics_lib)

add_test(NAME ballistics_tests COMMAND ballistics_tests)
