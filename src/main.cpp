#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "context.h"
#include "gui.h"
#include "renderer.h"

void FramebufferSizeCallback(GLFWwindow *Window, int Width, int Height);
void ProcessInput(context *Context);

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
    context Context = {.Window = Window};

    if (Context.Window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Context.Window);
    glfwSetFramebufferSizeCallback(Context.Window, FramebufferSizeCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    gui Gui = GuiCreate(&Context);
    renderer Renderer = RendererCreate();

    entity Entity = {.position = glm::vec3(0.0f, 0.0f, 0.0f)};
    Context.Entity = &Entity;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(Context.Window)) {
        // input
        // --------------------
        if (!Gui.io.WantCaptureMouse) {
            ProcessInput(&Context);
        }

        // gui: new frame
        // ------
        GuiNewFrame();

        // render
        // ------
        ClearBackground(0.1f, 0.1f, 0.1f, 1.0f);
        // DrawTriangle(&Renderer, Entity.position);
        // DrawRectangle(&Renderer, Entity.position);
        DrawCube(&Renderer, Entity.position);

        // gui
        // ------
        GuiDraw(&Context);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(Context.Window);
        glfwPollEvents();
    }

    GuiDestroy();
    RendererDestroy(&Renderer);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void ProcessInput(context *Context) {
    if (glfwGetKey(Context->Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(Context->Window, true);
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
