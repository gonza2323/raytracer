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
        // Generamos un punto aleatorio en un cubo de -1 a 1
        auto p = random_vector(-1, 1);
        auto lensq = glm::length(p) * glm::length(p); // longitud al cuadrado para ahorrar la raíz cuadrada inicial

        // El punto debe estar dentro de la esfera unitaria (lensq <= 1)
        // y no ser demasiado pequeño (para evitar errores de división por cero)
        if (lensq > 1e-160 && lensq <= 1.0)
            return glm::normalize(p); // Normalizamos para que la longitud sea exactamente 1
    }
}
