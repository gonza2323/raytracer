#pragma once

#include "ray.h"
#include "scene.h"
#include "tile.h"
#include <SDL3/SDL_stdinc.h>
#include <cmath>
#include <cstdint>
#include <glm/ext/vector_int3.hpp>

class Renderer {
public:
    Renderer(Scene& scene, int width, int no_samples);
    
    bool advance();

    int getWidth() { return width; }
    uint32_t* getPixels() { return pixels.data(); }
    
private:
    Scene& scene;
    
    int TILE_SIZE = 64;
    int width, height, no_samples;
    std::vector<uint32_t> pixels;
    std::vector<Tile> tileQueue;

    void generate_tiles();
    void process_tile(Tile& tile);
    void process_pixel(int x, int y);
    glm::vec3 shoot_ray(Ray& ray, int depth);
    glm::vec3 calculate_direct_lighting(HitData& hit_data);
    void write_pixel(int x, int y, glm::ivec3& pixel);
};