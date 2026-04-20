#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "camera.h"
#include "entity.h"
#include "scene.h"
#include <GLFW/glfw3.h>

struct context {
    GLFWwindow *Window;
    int ScreenWidth;
    int ScreenHeight;
    int FramebufferWidth;
    int FramebufferHeight;
    int ShadowbufferWidth;
    int ShadowbufferHeight;

    camera Camera;

    float DeltaTime;
    float LastFrame;

    float LastX;
    float LastY;
    bool FirstClick;

    int CurrentSceneIdx;
    std::vector<scene *> Scenes;
};

#endif
