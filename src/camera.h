#ifndef CAMERA_H_
#define CAMERA_H_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum camera_movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

typedef struct camera {
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
} camera;

camera Camera_Create(glm::vec3 Position, glm::vec3 Up, float Yaw, float Pitch);
glm::mat4 Camera_GetViewMatrix(const camera &Camera);
// processes input received from any keyboard-like input system. Accepts input
// parameter in the form of camera defined ENUM (to abstract it from windowing
// systems)
void Camera_ProcessKeyboard(camera *Camera, camera_movement Direction,
                            float DeltaTime);
// processes input received from a mouse input system. Expects the offset
// value in both the x and y direction.
void Camera_ProcessMouseMovement(camera *Camera, float OffsetX, float OffsetY,
                                 GLboolean ConstrainPitch);
void Camera_ProcessMouseScroll(camera *Camera, float OffsetY);
#endif
