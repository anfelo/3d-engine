#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "renderer.h"

void FramebufferSizeCallback(GLFWwindow *Window, int Width, int Height);
void ProcessInput(GLFWwindow *window);

// settings
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *Window =
        glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Engine", NULL, NULL);
    if (Window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Window);
    glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    renderer Renderer = RendererCreate();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(Window)) {
        // input
        // -----
        ProcessInput(Window);

        // render
        // ------
        ClearBackground(0.1f, 0.1f, 0.1f, 1.0f);
        // DrawTriangle(&Renderer, glm::vec3(0.0f, 0.0f, 0.0f));
        // DrawRectangle(&Renderer, glm::vec3(0.0f, 0.0f, 0.0f));
        DrawCube(&Renderer, glm::vec3(0.0f, 0.0f, 0.0f));

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(Window);
        glfwPollEvents();
    }

    RendererDestroy(&Renderer);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void ProcessInput(GLFWwindow *Window) {
    if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(Window, true);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void FramebufferSizeCallback(GLFWwindow *Window, int Width, int Height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, Width, Height);
}
