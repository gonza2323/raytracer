#include <SDL3/SDL.h>
#include <stdint.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main()
{
    glm::mat4 m(1.0f);
    auto t = glm::translate(m, glm::vec3(1, 0, 0));
    
    SDL_Init(SDL_INIT_VIDEO);

    const int WIDTH = 800;
    const int HEIGHT = 600;

    SDL_Window* window = SDL_CreateWindow(
        "Raytracer",
        WIDTH,
        HEIGHT,
        0
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // Texture that we can update every frame
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    // CPU framebuffer
    uint32_t* pixels = new uint32_t[WIDTH * HEIGHT];

    bool running = true;

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }

        // Example: animate colors
        static int frame = 0;
        frame++;

        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                uint8_t r = (x + frame) % 256;
                uint8_t g = (y + frame) % 256;
                uint8_t b = 128;

                pixels[y * WIDTH + x] =
                    (255 << 24) | // A
                    (r << 16) |
                    (g << 8)  |
                    b;
            }
        }

        // Upload framebuffer to GPU texture
        SDL_UpdateTexture(
            texture,
            NULL,
            pixels,
            WIDTH * sizeof(uint32_t)
        );

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    delete[] pixels;

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}