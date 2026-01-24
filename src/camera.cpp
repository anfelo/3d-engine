#include "camera.h"

static void Camera_UpdateVectors(camera *Camera);

camera Camera_Create(glm::vec3 Position, glm::vec3 Up, float Yaw, float Pitch) {
    camera Camera = {};
    Camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Camera.MovementSpeed = SPEED;
    Camera.MouseSensitivity = SENSITIVITY;
    Camera.Zoom = ZOOM;
    Camera.Position = Position;
    Camera.WorldUp = Up;
    Camera.Yaw = Yaw;
    Camera.Pitch = Pitch;

    Camera_UpdateVectors(&Camera);

    return Camera;
}

glm::mat4 Camera_GetViewMatrix(camera *Camera) {
    return glm::lookAt(Camera->Position, Camera->Position + Camera->Front,
                       Camera->Up);
}

void Camera_ProcessKeyboard(camera *Camera, camera_movement Direction,
                            float DeltaTime) {
    float Velocity = Camera->MovementSpeed * DeltaTime;

    if (Direction == FORWARD) {
        Camera->Position += Camera->Front * Velocity;
    }
    if (Direction == BACKWARD) {
        Camera->Position -= Camera->Front * Velocity;
    }
    if (Direction == LEFT) {
        Camera->Position -= Camera->Right * Velocity;
    }
    if (Direction == RIGHT) {
        Camera->Position += Camera->Right * Velocity;
    }
    if (Direction == UP) {
        Camera->Position += Camera->Up * Velocity;
    }
    if (Direction == DOWN) {
        Camera->Position -= Camera->Up * Velocity;
    }
}

void Camera_ProcessMouseMovement(camera *Camera, float OffsetX, float OffsetY,
                                 GLboolean ConstrainPitch) {
    OffsetX *= Camera->MouseSensitivity;
    OffsetY *= Camera->MouseSensitivity;

    Camera->Yaw += OffsetX;
    Camera->Pitch += OffsetY;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (ConstrainPitch) {
        if (Camera->Pitch > 89.0f) {
            Camera->Pitch = 89.0f;
        }
        if (Camera->Pitch < -89.0f) {
            Camera->Pitch = -89.0f;
        }
    }

    Camera_UpdateVectors(Camera);
}

void Camera_ProcessMouseScroll(camera *Camera, float OffsetY) {
    Camera->Zoom -= (float)OffsetY;
    if (Camera->Zoom < 1.0f) {
        Camera->Zoom = 1.0f;
    }
    if (Camera->Zoom > 45.0f) {
        Camera->Zoom = 45.0f;
    }
}

static void Camera_UpdateVectors(camera *Camera) {
    // calculate the new Front vector
    glm::vec3 Front;
    Front.x = cos(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
    Front.y = sin(glm::radians(Camera->Pitch));
    Front.z = sin(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
    Camera->Front = glm::normalize(Front);

    // also re-calculate the Right and Up vector
    // normalize the vectors, because their length gets
    // closer to 0 the more you look up or down which
    // results in slower movement.
    Camera->Right = glm::normalize(glm::cross(Camera->Front, Camera->WorldUp));
    Camera->Up = glm::normalize(glm::cross(Camera->Right, Camera->Front));
}
