#include "renderer.h"
#include "hit_data.h"
#include "lights.h"
#include "ray.h"
#include "color.h"
#include <algorithm>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

#include "util.h"
#include "constants.h"

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
    glm::vec3 light(0.0f);
    
    int no_samples = 30;
    Ray ray = scene.camera.generateRayForPixel(x, y, width, height);
    for (int i = 0; i < no_samples; i++) {
        glm::vec3 sample = shoot_ray(ray, 8);
        light += sample;
    }

    light /= no_samples;
    glm::ivec3 pixel = process_color(light);
    write_pixel(x, y, pixel);
}

glm::vec3 Renderer::shoot_ray(Ray& ray, int depth)
{
    if (depth <= 0) return glm::vec3(0.0f);

    HitData hit_data;

    // si no hay hit, retornamos color de fondo
    if (!scene.intersect(ray, hit_data))
        return glm::vec3(0.0f);

    glm::vec3 bounce_direction = random_on_hemisphere(hit_data.normal);
    Ray scattered_ray(hit_data.pos + hit_data.normal * 0.001f, bounce_direction);
    
    glm::vec3 surface_color = scene.materials[hit_data.material_index].color;

    glm::vec3 direct_lighting = calculate_direct_lighting(hit_data);
    
    float cosine_factor = std::max(glm::dot(bounce_direction, hit_data.normal), 0.0f);

    glm::vec3 indirect_light = shoot_ray(scattered_ray, depth - 1);
    glm::vec3 indirect_lighting = indirect_light * cosine_factor;

    return (direct_lighting + indirect_lighting) * surface_color;
}

glm::vec3 Renderer::calculate_direct_lighting(HitData& hit_data) {
    glm::vec3 total_light(0.0f);

    for (Light* light : scene.lights) {
        glm::vec3 light_dir = light->get_direction(hit_data.pos);
        glm::vec3 light_dir_normalized = glm::normalize(light_dir);

        float cosine_factor = std::max(glm::dot(-light_dir_normalized, hit_data.normal), 0.0f);

        if (cosine_factor <= 0)
            continue;

        // Create shadow ray from hit point towards the light
        Ray shadow_ray{hit_data.pos + hit_data.normal * 0.001f, -light_dir};

        // Check if light is visible
        bool is_visible = false;

        // TODO esto está horrible, lo hizo claudio

        // For PointLight, only check occlusion up to the light distance
        if (PointLight* point_light = dynamic_cast<PointLight*>(light)) {
            float distance_to_light = glm::length(light_dir);
            is_visible = !scene.is_occluded(shadow_ray, distance_to_light);
        }
        // For DirectionalLight, check occlusion to infinity
        else if (dynamic_cast<DirectionalLight*>(light)) {
            is_visible = !scene.is_occluded(shadow_ray, infinity);
        }

        if (is_visible) {
            glm::vec3 light_intensity = light->get_intensity_at(hit_data.pos);
            total_light += light_intensity * cosine_factor;
        }
    }

    return total_light;
}

void Renderer::write_pixel(int x, int y, glm::ivec3& pixel)
{
    pixels[y * width + x] =
        (255 << 24) | // alpha channel
        (pixel.r << 16) |
        (pixel.g << 8) |
        pixel.b;
}
