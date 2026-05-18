#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <stdint.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "lights.h"
#include "scene.h"
#include "renderer.h"
#include "scene_loader.h"


int main(int argc, char* argv[])
{
    // INICIALIZAR ESCENA

    // dimensiones de la imagen
    int width;
    int height = 520;

    // cámara
    glm::vec3 camera_pos({-2.44,6.66,5.87});
    glm::vec3 camera_rot({0.0,0.0,0.0});
    float camera_focal_length = 30 * 0.001;   // 30 mm
    float camera_sensor_size_x = 36 * 0.001;  // 36 mm
    float camera_sensor_size_y = 24 * 0.001;  // 24 mm

    Camera camera(camera_pos, camera_rot, camera_focal_length,camera_sensor_size_x, camera_sensor_size_y);
    
    // escena
    Scene scene = Scene(camera);

    // cargar escena de prueba
    std::string file_path = "assets/Test.glb";
    SceneLoader::load_from_path(scene, file_path);

    // agregar luces
    PointLight light{glm::vec3(50.0f), glm::vec3(1.0f, 10.0f, 5.0f)};
    scene.lights.push_back(&light);

    // renderizador
    int no_samples = 30;
    Renderer renderer(scene, height, no_samples);
    width = renderer.getWidth();

    
    // INICIALIZAR GUI

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Raytracer",
        width,
        height,
        0
    );

    SDL_Renderer* sdl_renderer = SDL_CreateRenderer(window, NULL);

    // Texture that we can update every frame
    SDL_Texture* texture = SDL_CreateTexture(
        sdl_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );


    // LOOP DE LA INTERFAZ GRÁFICA

    bool running = true;
    bool completed = false;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_EVENT_QUIT)
                running = false;

        if (!completed) {
            bool tiles_left = renderer.advance();
            completed = !tiles_left;
            
            // Actualizar la imagen 
            SDL_UpdateTexture(
                texture,
                NULL,
                renderer.getPixels(),
                width * sizeof(uint32_t)
            );
            
            SDL_RenderClear(sdl_renderer);
            SDL_RenderTexture(sdl_renderer, texture, NULL, NULL);
            SDL_RenderPresent(sdl_renderer);
        }

        if (completed)
            SDL_Delay(50);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
