#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

static inline glm::vec3 toneMap(const glm::vec3& c)
{
    return c / (1.0f + c);
}

static inline glm::vec3 gammaCorrect(const glm::vec3& c)
{
    float invGamma = 1.0f / 2.2f;
    return glm::pow(c, glm::vec3(invGamma));
}

static inline glm::vec3 toLinear(const glm::vec3& c)
{
    return glm::pow(c, glm::vec3(2.2f));
}

static inline glm::ivec3 process_color(const glm::vec3& color)
{
    glm::vec3 mapped = toneMap(color);
    glm::vec3 gamma = gammaCorrect(mapped);
    glm::vec3 clamped = glm::clamp(gamma, 0.0f, 1.0f);
    glm::ivec3 rounded = glm::round(clamped * 255.0f);

    return rounded;
}