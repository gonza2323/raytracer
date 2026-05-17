#pragma once

#include <glm/ext/vector_float3.hpp>

struct Light {
    glm::vec3 color;
    virtual glm::vec3 get_intensity_at(glm::vec3& point) = 0;
    virtual glm::vec3 get_direction(glm::vec3& point) = 0;

protected:
    Light(glm::vec3 color)
        :color(color) { };
};

struct DirectionalLight : Light {
    glm::vec3 dir;

    DirectionalLight(glm::vec3 color, glm::vec3 dir)
        :Light(color), dir(dir) { }

    glm::vec3 get_intensity_at(glm::vec3& point) override;
    glm::vec3 get_direction(glm::vec3& point) override;
};

struct PointLight : Light {
    glm::vec3 pos;
  
    PointLight(glm::vec3 color, glm::vec3 pos)
        :Light(color), pos(pos) { }

    glm::vec3 get_intensity_at(glm::vec3& point) override;
    glm::vec3 get_direction(glm::vec3& point) override;
};
