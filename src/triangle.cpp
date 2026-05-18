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

    // Barycentric coordinates: u for v1, v for v2, w for v0
    float w = 1.0f - u - v;

    // Interpolate normal
    hit_data.shading_normal = glm::normalize(w * v0.normal + u * v1.normal + v * v2.normal);

    // Interpolate UVs
    for (int i = 0; i < 2; ++i) {
        hit_data.uvs[i] = w * v0.uvs[i] + u * v1.uvs[i] + v * v2.uvs[i];
    }

    glm::vec3 geometric_normal = glm::normalize(glm::cross(edge1, edge2));

    // Optional: make normals face against the ray
    if (glm::dot(geometric_normal, ray.dir) > 0.0f) {
        geometric_normal = -geometric_normal;
    }

    hit_data.normal = geometric_normal;
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