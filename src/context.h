#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "camera.h"
#include "entity.h"
#include <GLFW/glfw3.h>

struct context {
    GLFWwindow *Window;
    float ScreenWidth;
    float ScreenHeight;

    camera *Camera;
    entity *Entity;
    light_entity *LightEntity;

    float DeltaTime;
    float LastFrame;

    float LastX;
    float LastY;
    bool FirstClick;
};

#endif
