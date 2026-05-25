#pragma once

#include "bvh/bounding_box.h"
#include "hit_data.h"
#include "ray.h"
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uvs[2];
};

struct Triangle
{
    Vertex v0, v1, v2;
    int material_index;

    BoundingBox generate_bounding_box();

    bool intersect(Ray ray, HitData& hit_data);
};
