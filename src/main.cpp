#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <mpi.h>

#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <iostream>

#include "lights.h"
#include "scene.h"
#include "renderer.h"
#include "scene_loader.h"
#include "mpi/mpi_scheduler.h"
#include <chrono>


int main(int argc, char* argv[])
{
    auto start_time = std::chrono::high_resolution_clock::now();
    // PARSE PROGRAM ARGUMENTS

    // Default values
    int height = 520;
    std::string scene_path = "assets/Test.glb";
    int no_samples = 30;
    std::string output_path = "output.png";

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if ((arg == "-h" || arg == "--height") && i + 1 < argc)
        {
            height = std::stoi(argv[++i]);
        }
        else if ((arg == "-s" || arg == "--scene") && i + 1 < argc)
        {
            scene_path = argv[++i];
        }
        else if ((arg == "-n" || arg == "--samples") && i + 1 < argc)
        {
            no_samples = std::stoi(argv[++i]);
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc)
        {
            output_path = argv[++i];
        }
    }

    MPI_Init(&argc, &argv);

    int world_rank = 0;
    int world_size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Initialize scene

    glm::vec3 camera_pos({-2.44, 6.66, 5.87});
    glm::vec3 camera_rot({0.0, 0.0, 0.0});
    float camera_focal_length = 30 * 0.001f; // 30 mm
    float camera_sensor_size_x = 36 * 0.001f; // 36 mm
    float camera_sensor_size_y = 24 * 0.001f; // 24 mm

    Camera camera(camera_pos, camera_rot, camera_focal_length, camera_sensor_size_x, camera_sensor_size_y);
    Scene scene = Scene(camera);

    SceneLoader::load_from_path(scene, scene_path);

    DirectionalLight light{glm::vec3(500.0f), glm::vec3(-1.0f, -10.0f, 1.0f)};
    scene.lights.push_back(&light);

    Renderer renderer(scene, height, no_samples);

    if (world_rank == 0)
    {
        std::vector<uint32_t> framebuffer;
        mpi_scheduler::MasterCallbacks callbacks{};

        bool completed = mpi_scheduler::run_master_render(renderer, world_size, framebuffer, callbacks);
        if (completed)
        {
            int width = renderer.getWidth();
            stbi_write_png(output_path.c_str(), width, height, 4, framebuffer.data(), width * sizeof(uint32_t));

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << "Elapsed time: " << duration.count() / 1000.0f << " s\n";
        }
    }
    else
    {
        mpi_scheduler::run_worker_render(renderer);
    }

    MPI_Finalize();
    return 0;
}