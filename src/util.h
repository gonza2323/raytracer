#pragma once
#include <random>
#include <glm/vec3.hpp>
#include <glm/ext/quaternion_geometric.hpp>

// generate a random number in [0, 1)
inline double random_double()
{
    static std::uniform_real_distribution<double> dist(0, 1);
    static std::mt19937 generator;
    return dist(generator);
}

// generate a real random number in [min, max)
inline double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}


static glm::vec3 random_vector()
{
    return glm::vec3(random_double(), random_double(), random_double());
}

inline glm::vec3 random_vector(double min, double max)
{
    return glm::vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

inline glm::vec3 random_unit_vector()
{
    while (true)
    {
        // Pick a random point in the unit cube and keep those inside the unit sphere.
        auto p = random_vector(-1, 1);
        auto lensq = glm::length(p) * glm::length(p);

        if (lensq > 1e-160 && lensq <= 1.0)
            return glm::normalize(p);
    }
}

inline glm::vec3 random_on_hemisphere(glm::vec3& normal)
{
    glm::vec3 on_unit_sphere = random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}