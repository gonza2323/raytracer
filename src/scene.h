#pragma once

#include <vector>
#include <string>

#include "camera.h"
#include "hit_data.h"
#include "ray.h"
#include "lights.h"
#include "triangle.h"
#include "bvh/bvh_node.h"

struct Image
{
    int width, height, channels;
    std::vector<unsigned char> data;
};

struct Scene
{
    Camera camera;
    std::vector<Triangle> triangles;
    std::vector<Light*> lights;
    std::vector<Material> materials;
    std::vector<Image> textures;
    BVHNode* bvh_root = nullptr;

    Scene(Camera camera)
        : camera(camera)
    {
    }

    ~Scene()
    {
        delete bvh_root;
    }

    bool intersect(Ray ray, HitData& hit_data);
    bool is_occluded(Ray ray, float max_t);
    void build_bvh();
};