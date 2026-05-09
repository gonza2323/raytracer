cmake_minimum_required(VERSION 3.28)

project(raytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(raytracer
    src/main.cpp
)

find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(raytracer
    PRIVATE
        SDL3::SDL3
)
