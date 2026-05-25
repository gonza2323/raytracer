#pragma once

#include "hit_data.h"
#include "material.h"
#include "ray.h"
#include <glm/ext/vector_float3.hpp>

struct Sphere
{
    glm::vec3 pos;
    float radius;

    int material_index;

    bool intersect(Ray ray, HitData& hit_data);
};
