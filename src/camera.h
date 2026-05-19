#pragma once

#include <glm/ext/vector_float3.hpp>
#include "ray.h"

struct Camera {
    glm::vec3 pos;
    glm::vec3 rot; // rotacion en Euler: pitch, yaw, roll

    float focal_length;
    float size_x, size_y; // tamaño del sensor en metros

    Camera(glm::vec3 pos, glm::vec3 rot, float focal_length, float size_x, float size_y)
        : pos(pos), rot(rot), focal_length(focal_length), size_x(size_x), size_y(size_y) { }

    float getAspectRatio() {
        return size_x / size_y;
    }

    // Devuelve el rayo que sale de esta camara hacia el pixel (x, y)
    Ray generateRayForPixel(int x, int y, int image_width, int image_height);
    Ray generateRayForPixelAA(int x, int y, int image_width, int image_height);
};