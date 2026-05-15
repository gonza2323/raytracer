#include "triangle.h"
#include <glm/geometric.hpp>

bool Triangle::intersect(Ray ray, HitData& hit_data) {
    constexpr float EPSILON = 1e-6f;

    glm::vec3 edge1 = v1.pos - v0.pos;
    glm::vec3 edge2 = v2.pos - v0.pos;

    glm::vec3 h = glm::cross(ray.dir, edge2);
    float a = glm::dot(edge1, h);

    // Ray parallel to triangle
    if (fabs(a) < EPSILON) {
        return false;
    }

    float f = 1.0f / a;

    glm::vec3 s = ray.origin - v0.pos;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray.dir, q);

    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    float t = f * glm::dot(edge2, q);

    // Triangle is behind ray
    if (t <= EPSILON) {
        return false;
    }

    hit_data.t = t;
    hit_data.pos = ray.origin + ray.dir * t;

    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

    // Optional: make normals face against the ray
    if (glm::dot(normal, ray.dir) > 0.0f) {
        normal = -normal;
    }

    hit_data.normal = normal;
    hit_data.material_index = material_index;

    return true;
}

BoundingBox Triangle::generate_bounding_box() {
    const glm::vec3 min_point = glm::min(
        glm::min(v0.pos, v1.pos),
        v2.pos
    );

    const glm::vec3 max_point = glm::max(
        glm::max(v0.pos, v1.pos),
        v2.pos
    );

    return BoundingBox(min_point, max_point);
}