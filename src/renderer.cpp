#include "renderer.h"
#include "camera.h"
#include "hit_data.h"
#include "ray.h"
#include "color.h"
#include <algorithm>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

Renderer::Renderer(Scene& scene, int width)
    : scene(scene),
      width(width)
{
    float cameraAspectRatio = scene.camera.getAspectRatio();
    this->height = std::round(width / cameraAspectRatio);
    
    pixels.resize(width * height);

    generateTiles();
}

bool Renderer::advance() {
    if (tileQueue.empty())
        return false;
    
    Tile tile = tileQueue.back();
    processTile(tile);
    
    tileQueue.pop_back();
    return true;
}

void Renderer::generateTiles() {
    for (int y = 0; y < height; y += TILE_SIZE) {
        int y_end = std::min(y + TILE_SIZE, height);
        
        for (int x = 0; x < width; x += TILE_SIZE) {
            int x_end = std::min(x + TILE_SIZE, width);
            
            Tile tile = {x, x_end, y, y_end};
            tileQueue.push_back(tile);
        }
    }
}

void Renderer::processTile(Tile& tile) {
    for (int y = tile.y_start; y < tile.y_end; y++) {
        for (int x = tile.x_start; x < tile.x_end; x++) {
            processPixel(x, y);
        }
    }
}

void Renderer::processPixel(int x, int y) {
    // TODO: Deberíamos hacerlo desde la perspectiva de la cámara
    // glm::vec3 ray = scene.camera.generateRayForPixel(x, y);

    // TODO revisar bien este cálculo
    float sensor_pos_x_rel = (float)x / (width-1) - 0.5;
    float sensor_pos_y_rel = -((float)y / (height-1) - 0.5);

    float sensor_pos_x = sensor_pos_x_rel * scene.camera.size_x;
    float sensor_pos_y = sensor_pos_y_rel * scene.camera.size_y;
    
    glm::vec3 ray_direction = glm::vec3(sensor_pos_x, sensor_pos_y, -scene.camera.focal_length);
    glm::vec3 ray_origin(scene.camera.pos);

    Ray ray = {ray_origin, ray_direction};
    
    HitData hit_data;
    bool hit = scene.intersect(ray, hit_data);

    glm::vec3 background_color(0.05f);
    glm::vec3 color = background_color;
    if (hit) {
        // TODO Por ahora solo una luz direccional, pero habría que usar las luces de la escena.
        glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, -2.0, -1.0));
        glm::vec3 light_color = glm::vec3(1.0, 1.0, 1.0);
        float lighting_factor = std::max(glm::dot(-light_dir, hit_data.normal), 0.0f);
        color = lighting_factor * light_color * hit_data.color;
    }

    glm::ivec3 pixel = toPixel(color);
    write_pixel(x, y, pixel);
}

void Renderer::write_pixel(int x, int y, glm::ivec3 pixel) {
    pixels[y * width + x] =
        (255 << 24) | // alpha channel
        (pixel.r << 16) |
        (pixel.g << 8)  |
        pixel.b;
}
