#pragma once

#include "material.h"
#include <glm/ext/vector_float3.hpp>


struct HitData {
    glm::vec3 pos;
    glm::vec3 normal;
    
    float t;

    int material_index;
};