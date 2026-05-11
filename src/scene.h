#pragma once

#include <vector>
#include <string>

#include "camera.h"
#include "hit_data.h"
#include "ray.h"
#include "lights/light.h"
#include "triangle.h"

struct Scene {
    Camera camera;
    std::vector<Triangle> triangles;
    std::vector<Light*> lights;
    std::vector<Material> materials;

    Scene(Camera camera)
        :camera(camera) { }
    
    bool intersect(Ray ray, HitData& hit_data);
};


void load_scene_from_path(Scene& scene, std::string& file_path);