#include "camera.h"
#include "util.h"
#include <glm/gtc/matrix_transform.hpp>

static Ray generateRayForSensorPos(const Camera& cam, float x, float y, int image_width, int image_height)
{
    // Map (x, y) to normalized sensor coordinates; invert Y because image origin is top-left.
    float u = (float)x / (image_width - 1) - 0.5f; // [-0.5, 0.5]
    float v = -((float)y / (image_height - 1) - 0.5f); // [0.5, -0.5]

    // Sensor position in world units.
    float sensor_x = u * cam.size_x;
    float sensor_y = v * cam.size_y;

    // Sensor is focal_length units in front of the camera (local -Z).
    // Direction in camera-local space.
    glm::vec3 local_dir = glm::vec3(sensor_x, sensor_y, -cam.focal_length);

    // Rotation matrix from Euler angles (Yaw - Pitch - Roll).
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, cam.rot.y, glm::vec3(0, 1, 0)); // Yaw   (izq/der)
    rotation = glm::rotate(rotation, cam.rot.x, glm::vec3(1, 0, 0)); // Pitch (arriba/abajo)
    rotation = glm::rotate(rotation, cam.rot.z, glm::vec3(0, 0, 1)); // Roll  (inclinar)

    // Transform to world direction.
    glm::vec3 world_dir = glm::vec3(rotation * glm::vec4(local_dir, 0.0f));

    // Build the ray.
    Ray ray;
    ray.origin = cam.pos;
    ray.dir = world_dir;

    return ray;
}

// Exact pixel ray.
Ray Camera::generateRayForPixel(int x, int y, int image_width, int image_height)
{
    return generateRayForSensorPos(*this, (float)x, (float)y, image_width, image_height);
}

// Jittered ray for anti-aliasing.
Ray Camera::generateRayForPixelAA(int x, int y, int image_width, int image_height)
{
    float jitter_x = x + (float)random_double(-0.5, 0.5);
    float jitter_y = y + (float)random_double(-0.5, 0.5);
    return generateRayForSensorPos(*this, jitter_x, jitter_y, image_width, image_height);
}