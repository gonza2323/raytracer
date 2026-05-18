#include "scene.h"
#include "hit_data.h"
#include "ray.h"
#include "lights.h"
#include "triangle.h"
#include "bvh/bvh_node.h"
#include "constants.h"
#include "interval.h"
#include <cmath>
#include <filesystem>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>
#include <iostream>

bool Scene::intersect(Ray ray, HitData& hit_data) {
    if (!bvh_root) {
        return false;
    }

    // Use Interval to track valid ray range
    Interval ray_t(0.001, infinity);
    return bvh_root->intersect(ray, hit_data, triangles, ray_t);
}

bool Scene::is_occluded(Ray ray, float max_t) {
    if (!bvh_root) {
        return false;
    }

    HitData hit_data;
    Interval ray_t(0.001, max_t);
    return bvh_root->intersect(ray, hit_data, triangles, ray_t);
}

void Scene::build_bvh() {
    if (triangles.empty()) {
        return;
    }

    bvh_root = BVHNode::build(triangles, 0, triangles.size());
}