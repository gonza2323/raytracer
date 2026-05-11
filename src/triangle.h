#pragma once

#include "hit_data.h"
#include "material.h"
#include "ray.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Triangle {
    Vertex v0, v1, v2;
    int material_index;

    bool intersect(Ray ray, HitData& hit_data);
};