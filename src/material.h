#pragma once

#include <glm/vec3.hpp>

struct TextureInfo {
    int texture_index = -1;
    int uv_index = 0;
};

struct Material {
    glm::vec3 base_color_factor = glm::vec3(1.0f);
    float roughness_factor = 1.0f;
    float metallic_factor = 1.0f;

    TextureInfo base_color_texture;
    TextureInfo metallic_roughness_texture;
    TextureInfo normal_texture;
};
