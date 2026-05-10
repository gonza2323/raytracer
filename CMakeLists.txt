cmake_minimum_required(VERSION 3.28)

project(raytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(raytracer
    src/main.cpp
    src/renderer.cpp
    src/camera.cpp
    src/scene.cpp
    src/sphere.cpp
    src/third_party/stb_image_impl.cpp
)


# Dependencies

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)

target_link_libraries(raytracer
    PRIVATE
        SDL3::SDL3
        glm::glm
        fastgltf::fastgltf
)
