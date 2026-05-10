#include "scene.h"
#include "hit_data.h"
#include "sphere.h"
#include <cmath>
#include <glm/geometric.hpp>

bool Scene::intersect(Ray ray, HitData& hit_data) {
    bool hit = false;
    float closest_t = MAXFLOAT;

    HitData temp_hit;

    for (Sphere& sphere : spheres) {
        if (sphere.intersect(ray, temp_hit)) {
            hit = true;
            if (temp_hit.t < closest_t) {
                closest_t = temp_hit.t;
                hit_data = temp_hit;
            }
        }
    }

    return hit;
}
