#pragma once

#include <glm/ext/vector_float3.hpp>

struct Light {
    glm::vec3 color;
};

struct DirectionalLight : Light {
    glm::vec3 dir;
};

struct PointLight : Light {
    glm::vec3 pos;
};
