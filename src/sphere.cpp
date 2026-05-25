#include "sphere.h"
#include <glm/ext/quaternion_geometric.hpp>

bool Sphere::intersect(Ray ray, HitData& hit_data)
{
    const float EPS = 1e-4f;

    glm::vec3 oc = ray.origin - pos;

    float a = glm::dot(ray.dir, ray.dir);
    float b = 2.0f * glm::dot(oc, ray.dir);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f)
        return false;

    float sqrt_d = sqrt(discriminant);

    float t0 = (-b - sqrt_d) / (2.0f * a);
    float t1 = (-b + sqrt_d) / (2.0f * a);

    // ensure t0 is smaller
    if (t0 > t1) std::swap(t0, t1);

    float t = t0;

    // if closest is behind, try the far one (handles being inside sphere too)
    if (t < EPS)
    {
        t = t1;
        if (t < EPS) return false;
    }

    hit_data.pos = ray.origin + t * ray.dir;
    hit_data.material_index = material_index;
    hit_data.normal = glm::normalize(hit_data.pos - pos);
    hit_data.shading_normal = hit_data.normal;
    hit_data.uvs[0] = glm::vec2(0.0f);
    hit_data.uvs[1] = glm::vec2(0.0f);
    hit_data.t = t;

    return true;
}