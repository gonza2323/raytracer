#include "renderer.h"
#include "hit_data.h"
#include "lights.h"
#include "ray.h"
#include "color.h"
#include <algorithm>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <numbers>

#include "util.h"
#include "constants.h"

Renderer::Renderer(Scene& scene, int height, int no_samples)
    : scene(scene),
      height(height),
      no_samples(no_samples)
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

void Renderer::render_tile(const Tile& tile, std::vector<uint32_t>& out_pixels)
{
    process_tile(tile);

    int tile_width = tile.x_end - tile.x_start;
    int tile_height = tile.y_end - tile.y_start;
    out_pixels.resize(tile_width * tile_height);

    for (int y = 0; y < tile_height; ++y)
    {
        const uint32_t* src = pixels.data() + (tile.y_start + y) * width + tile.x_start;
        uint32_t* dst = out_pixels.data() + y * tile_width;
        std::copy(src, src + tile_width, dst);
    }
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
            total_tiles++;
        }
    }
}

void Renderer::process_tile(const Tile& tile)
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

    for (int i = 0; i < no_samples; i++)
    {
        Ray ray = scene.camera.generateRayForPixelAA(x, y, width, height);
        light += shoot_ray(ray, 4);
    }

    light /= no_samples;
    glm::ivec3 pixel = process_color(light);
    write_pixel(x, y, pixel);
}

glm::vec3 Renderer::shoot_ray(
    Ray& ray,
    int depth)
{
    if (depth <= 0)
        return glm::vec3(0.0f);

    HitData hit_data;

    // Environment
    if (!scene.intersect(ray, hit_data))
    {
        return glm::vec3(0.0f);
    }

    resolve_surface_data(hit_data);

    glm::vec3 albedo =
        hit_data.color;

    // Direct lighting
    glm::vec3 direct =
        calculate_direct_lighting(hit_data)
        * albedo;

    // Cosine-weighted diffuse bounce
    glm::vec3 bounce_direction =
        random_cosine_weighted_direction(
            hit_data.shading_normal
        );

    Ray scattered_ray(
        hit_data.pos +
        hit_data.normal * 0.001f,
        bounce_direction
    );

    glm::vec3 incoming_indirect =
        shoot_ray(scattered_ray, depth - 1);

    // Cosine + PDF + BRDF cancel:
    // indirect = incoming * albedo

    glm::vec3 indirect =
        incoming_indirect * albedo;

    return direct + indirect;
}

void Renderer::resolve_surface_data(HitData& hit_data)
{
    const Material& mat = scene.materials[hit_data.material_index];

    // Base Color
    hit_data.color = mat.base_color_factor;
    if (mat.base_color_texture.texture_index != -1)
    {
        glm::vec3 tex_color = sample_texture(mat.base_color_texture.texture_index,
                                             hit_data.uvs[mat.base_color_texture.uv_index]);
        hit_data.color *= toLinear(tex_color);
    }

    // Roughness & Metallic
    hit_data.roughness = mat.roughness_factor;
    hit_data.metallic = mat.metallic_factor;
    if (mat.metallic_roughness_texture.texture_index != -1)
    {
        glm::vec3 tex_rm = sample_texture(mat.metallic_roughness_texture.texture_index,
                                          hit_data.uvs[mat.metallic_roughness_texture.uv_index]);
        // Green channel: roughness, Blue channel: metallic
        hit_data.roughness *= tex_rm.g;
        hit_data.metallic *= tex_rm.b;
    }

    // Normal Map
    hit_data.normal_map_normal = glm::vec3(0.0f, 0.0f, 1.0f); // Default tangent-space normal
    if (mat.normal_texture.texture_index != -1)
    {
        glm::vec3 tex_n = sample_texture(mat.normal_texture.texture_index, hit_data.uvs[mat.normal_texture.uv_index]);
        hit_data.normal_map_normal = glm::normalize(tex_n * 2.0f - 1.0f);
    }
}

glm::vec3 Renderer::calculate_direct_lighting(
    HitData& hit_data)
{
    glm::vec3 total_light(0.0f);

    for (Light* light : scene.lights)
    {
        glm::vec3 light_dir =
            light->get_direction(hit_data.pos);

        glm::vec3 L =
            glm::normalize(-light_dir);

        float NdotL =
            glm::max(
                glm::dot(
                    hit_data.shading_normal,
                    L
                ),
                0.0f
            );

        if (NdotL <= 0.0f)
            continue;

        Ray shadow_ray(
            hit_data.pos +
            hit_data.normal * 0.001f,
            L
        );

        bool visible = false;

        if (PointLight* point_light =
            dynamic_cast<PointLight*>(light))
        {
            float dist =
                glm::length(light_dir);

            visible =
                !scene.is_occluded(
                    shadow_ray,
                    dist
                );
        }
        else
        {
            visible =
                !scene.is_occluded(
                    shadow_ray,
                    infinity
                );
        }

        if (!visible)
            continue;

        glm::vec3 Li =
            light->get_intensity_at(hit_data.pos);

        // Lambertian BRDF:
        // albedo / PI

        total_light +=
            Li * NdotL / std::numbers::pi_v<float>;
    }

    return total_light;
}

glm::vec3 Renderer::sample_texture(int texture_index, glm::vec2 uv)
{
    if (texture_index < 0 || texture_index >= static_cast<int>(scene.textures.size()))
    {
        return glm::vec3(1.0f);
    }

    const Image& img = scene.textures[texture_index];
    if (img.data.empty()) return glm::vec3(1.0f);

    // Repeat wrapping
    float u = uv.x - std::floor(uv.x);
    float v = uv.y - std::floor(uv.y);

    int x = static_cast<int>(u * img.width);
    int y = static_cast<int>(v * img.height);

    // Clamp to avoid out of bounds due to floating point precision
    x = std::clamp(x, 0, img.width - 1);
    y = std::clamp(y, 0, img.height - 1);

    int pixel_index = (y * img.width + x) * 4;
    return glm::vec3(
        img.data[pixel_index + 0] / 255.0f,
        img.data[pixel_index + 1] / 255.0f,
        img.data[pixel_index + 2] / 255.0f
    );
}

void Renderer::write_pixel(int x, int y, glm::ivec3& pixel)
{
    pixels[y * width + x] =
        (255 << 24) |
        (pixel.b << 16) |
        (pixel.g << 8) |
        pixel.r;
}