#pragma once

#include <vector>

#include "camera.h"
#include "hit_data.h"
#include "ray.h"
#include "sphere.h"
#include "lights/light.h"

struct Scene {
    Camera camera;
    std::vector<Sphere> spheres;
    std::vector<Light*> lights;

    Scene(Camera camera)
        :camera(camera) { }
    
    bool intersect(Ray ray, HitData& hit_data);
};
