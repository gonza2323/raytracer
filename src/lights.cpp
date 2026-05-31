#include "lights.h"
#include <glm/geometric.hpp>

glm::vec3 DirectionalLight::get_intensity_at(glm::vec3& point) {
    return color;
}

glm::vec3 PointLight::get_intensity_at(glm::vec3& point) {
    float distance = glm::length(point - pos);
    float attenuation = 1.0f / (distance * distance);
    return color * attenuation;
}

glm::vec3 DirectionalLight::get_direction(glm::vec3& point) {
    return dir;
}

glm::vec3 PointLight::get_direction(glm::vec3& point) {
    return point - pos;
}