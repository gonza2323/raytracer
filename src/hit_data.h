#pragma once

#include "material.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/vec2.hpp>


struct HitData {
    glm::vec3 pos;
    glm::vec3 normal;         // Geometric normal (for offset/shadowing)
    glm::vec3 shading_normal; // Interpolated vertex normal
    glm::vec2 uvs[2];         // Interpolated UV coordinates
    
    // Resolved surface data
    glm::vec3 color;
    float roughness;
    float metallic;
    glm::vec3 normal_map_normal; // Normal sampled from map in tangent space (not used yet)

    float t;

    int material_index;
};
