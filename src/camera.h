#pragma once

#include <glm/ext/vector_float3.hpp>

struct Camera {
    glm::vec3 pos;
    glm::vec3 rot;

    float focal_length;
    float size_x, size_y;

    Camera(glm::vec3 pos, glm::vec3 rot, float focal_length, float size_x, float size_y)
        : pos(pos), rot(rot), focal_length(focal_length), size_x(size_x), size_y(size_y) { }
    
    float getAspectRatio() {
        return size_x / size_y;
    }

    glm::vec3 generateRayForPixel(int x, int y);
};
