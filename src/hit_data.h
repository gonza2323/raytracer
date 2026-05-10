#pragma once

#include <glm/ext/vector_float3.hpp>


struct HitData {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    float t;
};