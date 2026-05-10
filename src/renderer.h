#pragma once

#include "scene.h"
#include "tile.h"
#include <SDL3/SDL_stdinc.h>
#include <cmath>
#include <cstdint>
#include <glm/ext/vector_int3.hpp>

class Renderer {
public:
    Renderer(Scene& scene, int width);
    
    bool advance();

    int getHeight() { return height; }
    uint32_t* getPixels() { return pixels.data(); }
    
private:
    Scene& scene;
    
    int TILE_SIZE = 64;
    int width, height;
    std::vector<uint32_t> pixels;
    std::vector<Tile> tileQueue;

    void generateTiles();
    void processTile(Tile& tile);
    void processPixel(int x, int y);
    void write_pixel(int x, int y, glm::ivec3 pixel);
};