cmake_minimum_required(VERSION 3.28)

project(raytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(raytracer
    src/main.cpp
    src/mpi/mpi_protocol.cpp
    src/mpi/mpi_scheduler.cpp
    src/renderer.cpp
    src/camera.cpp
    src/scene.cpp
    src/scene_loader.cpp
    src/lights.cpp
    src/sphere.cpp
    src/third_party/stb_image_impl.cpp
    src/triangle.cpp
    src/bvh/bvh_node.cpp
)


# Dependencies

find_package(glm CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(MPI REQUIRED)

target_link_libraries(raytracer
    PRIVATE
        glm::glm
        fastgltf::fastgltf
        MPI::MPI_CXX
)


# Crear un symlink en el dir del ejecutable a assets/

file(CREATE_LINK
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    SYMBOLIC
)