#include <SDL3/SDL.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "SDL3",
        800,
        600,
        0
    );

    SDL_Delay(15000);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}