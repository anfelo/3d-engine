#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "entity.h"
#include <GLFW/glfw3.h>

struct context {
    GLFWwindow *Window;

    entity *Entity;
};

#endif
