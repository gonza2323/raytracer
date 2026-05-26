#pragma once
#include <fastgltf/math.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <numbers>
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

inline glm::vec3 random_cosine_direction()
{
    float r1 = random_double();
    float r2 = random_double();

    float phi = 2.0f * std::numbers::pi_v<float> * r1;

    float x = cos(phi) * sqrt(r2);
    float y = sin(phi) * sqrt(r2);
    float z = sqrt(1.0f - r2);

    return glm::vec3(x, y, z);
}

inline glm::vec3 random_cosine_weighted_direction(
    const glm::vec3& normal)
{
    glm::vec3 local = random_cosine_direction();

    glm::vec3 tangent;

    if (fabs(normal.x) > 0.9f)
    {
        tangent = glm::normalize(
            glm::cross(normal, glm::vec3(0,1,0))
        );
    }
    else
    {
        tangent = glm::normalize(
            glm::cross(normal, glm::vec3(1,0,0))
        );
    }

    glm::vec3 bitangent =
        glm::cross(normal, tangent);

    return glm::normalize(
        tangent   * local.x +
        bitangent * local.y +
        normal    * local.z
    );
}