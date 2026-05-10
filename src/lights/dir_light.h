#pragma once

#include "light.h"

struct DirectionalLight : Light {
    glm::vec3 dir;
};
