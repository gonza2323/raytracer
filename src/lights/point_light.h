#pragma once

#include <glm/ext/vector_float3.hpp>

#include "light.h"

struct PointLight : Light {
    glm::vec3 pos;
};
