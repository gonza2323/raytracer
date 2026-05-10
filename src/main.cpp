#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <cstdint>
#include <stdint.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scene.h"
#include "renderer.h"


int main(int argc, char* argv[])
{
    // INICIALIZAR ESCENA

    // dimensiones de la imagen
    int width = 800;
    int height;

    // cámara
    glm::vec3 camera_pos({0,0,30});
    glm::vec3 camera_rot(0.0f);
    float camera_focal_length = 40 * 0.001;   // 50 mm
    float camera_sensor_size_x = 36 * 0.001;  // 36 mm
    float camera_sensor_size_y = 24 * 0.001;  // 24 mm

    Camera camera(camera_pos, camera_rot, camera_focal_length,camera_sensor_size_x, camera_sensor_size_y);

    // escena
    Scene scene = Scene(camera);

    // esferas
    const int SPHERE_AMOUNT = 10;

    for (int i = 0; i < SPHERE_AMOUNT; i++) {

        glm::vec3 sphere_color{
            (float)rand() / RAND_MAX,
            (float)rand() / RAND_MAX,
            (float)rand() / RAND_MAX
        };

        glm::vec3 sphere_pos{
            ((float)rand() / RAND_MAX) * 10.0f - 5.0f,  // -5 to 5
            ((float)rand() / RAND_MAX) * 10.0f - 5.0f,  // -5 to 5
            ((float)rand() / RAND_MAX) * 10.0f - 5.0f,  // -5 to 5
        };

        float sphere_radius = 2.0f + ((float)rand() / RAND_MAX) * 1.0f;
        sphere_radius = 2.0f;

        scene.spheres.push_back({ sphere_pos, sphere_radius, sphere_color });
    }

    // renderizador
    Renderer renderer(scene, width);
    height = renderer.getHeight();

    
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
