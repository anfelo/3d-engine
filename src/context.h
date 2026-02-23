#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "camera.h"
#include "entity.h"
#include <GLFW/glfw3.h>

struct context {
    GLFWwindow *Window;
    int ScreenWidth;
    int ScreenHeight;
    int FramebufferWidth;
    int FramebufferHeight;

    camera Camera;

    float DeltaTime;
    float LastFrame;

    float LastX;
    float LastY;
    bool FirstClick;
};

#endif
