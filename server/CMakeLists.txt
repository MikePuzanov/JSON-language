cmake_minimum_required(VERSION 3.12)
project(server)

set(CMAKE_CXX_STANDARD 14)

# Добавьте путь к заголовочным файлам Crow
include_directories(crow)

add_executable(server/src/main.cpp)

# Добавьте необходимые зависимости и настройки CMake по мере необходимости
