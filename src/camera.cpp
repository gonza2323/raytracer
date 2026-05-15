#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

Ray Camera::generateRayForPixel(int x, int y, int image_width, int image_height) {

    // posicion normalizada del pixel en el sensor
    // mapear (x, y) a coordenadas [-0.5, 0.5] x [-0.5, 0.5]
    // La y se invierte porque en imagen y=0 es arriba
    float u = (float)x / (image_width  - 1) - 0.5f;  // [-0.5, 0.5]
    float v = -((float)y / (image_height - 1) - 0.5f); // [0.5, -0.5]

    // posición en el sensor en unidades de mundo
    float sensor_x = u * size_x;
    float sensor_y = v * size_y;

    // El sensor está a focal_length unidades adelante de la cámara (eje -Z local)
    // vector dirección en espacio local de la camara
    glm::vec3 local_dir = glm::vec3(sensor_x, sensor_y, -focal_length);

    // matriz de rotación a partir de los ángulos de Euler (rot)
    // Yaw - Pitch - Roll 
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, rot.y, glm::vec3(0, 1, 0)); // Yaw   (izq/der)
    rotation = glm::rotate(rotation, rot.x, glm::vec3(1, 0, 0)); // Pitch (arriba/abajo)
    rotation = glm::rotate(rotation, rot.z, glm::vec3(0, 0, 1)); // Roll  (inclinar)

    // transformar dir local
    glm::vec3 world_dir = glm::vec3(rotation * glm::vec4(local_dir, 0.0f));

    // armar el rayo
    Ray ray;
    ray.origin = pos;
    ray.dir    = world_dir;

    return ray;
}