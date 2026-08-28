cmake_minimum_required(VERSION 3.20)

project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(utf8-decoder
    src/main.cpp
    src/decoder.cpp
)

target_include_directories(utf8-decoder PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)