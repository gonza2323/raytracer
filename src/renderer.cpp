#include "renderer.h"
#include "hit_data.h"
#include "ray.h"
#include "color.h"
#include <algorithm>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

#include "util.h"

Renderer::Renderer(Scene& scene, int height)
    : scene(scene),
      height(height)
{
    float cameraAspectRatio = scene.camera.getAspectRatio();
    this->width = std::round(height * cameraAspectRatio);

    pixels.resize(width * height);

    generate_tiles();
}

bool Renderer::advance()
{
    if (tileQueue.empty())
        return false;

    Tile tile = tileQueue.back();
    process_tile(tile);

    tileQueue.pop_back();
    return true;
}

void Renderer::generate_tiles()
{
    for (int y = 0; y < height; y += TILE_SIZE)
    {
        int y_end = std::min(y + TILE_SIZE, height);

        for (int x = 0; x < width; x += TILE_SIZE)
        {
            int x_end = std::min(x + TILE_SIZE, width);

            Tile tile = {x, x_end, y, y_end};
            tileQueue.push_back(tile);
        }
    }
}

void Renderer::process_tile(Tile& tile)
{
    for (int y = tile.y_start; y < tile.y_end; y++)
    {
        for (int x = tile.x_start; x < tile.x_end; x++)
        {
            process_pixel(x, y);
        }
    }
}

void Renderer::process_pixel(int x, int y)
{
    // Esto que está acá debe ir en la clase Camera
    // que tenga un método generate_ray(int x, int y) que
    // retorne el rayo en la dirección correcta
    // también falta que tenga en cuenta la rotación de la cámara

    Ray ray = scene.camera.generateRayForPixel(x, y, width, height);
    glm::vec3 color = shoot_ray(ray, 100);
    glm::ivec3 pixel = process_color(color);
    write_pixel(x, y, pixel);
}

glm::vec3 Renderer::shoot_ray(Ray& ray, int depth)
{
    if (depth <= 0) return glm::vec3(0.0f);

    HitData hit_data;

    // si no hay hit, retornamos color de fondo
    if (!scene.intersect(ray, hit_data))
        return glm::vec3(0.5f);

    glm::vec3 bounce_direction = hit_data.normal + random_unit_vector();
    Ray scattered_ray(hit_data.pos + hit_data.normal * 0.001f, bounce_direction);
    glm::vec3 attenuation = scene.materials[hit_data.material_index].color;

    // TODO Por ahora solo una luz direccional, pero habría que usar las luces de la escena.
    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, -2.0, -1.0));
    glm::vec3 light_color = glm::vec3(1.0, 1.0, 1.0);
    float lighting_factor = std::max(glm::dot(-light_dir, hit_data.normal), 0.0f);
    return lighting_factor * light_color * attenuation * shoot_ray(scattered_ray, depth - 1);
}

void Renderer::write_pixel(int x, int y, glm::ivec3& pixel)
{
    pixels[y * width + x] =
        (255 << 24) | // alpha channel
        (pixel.r << 16) |
        (pixel.g << 8) |
        pixel.b;
}
