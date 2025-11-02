#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "camera.h"
#include "context.h"
#include "gui.h"
#include "renderer.h"

void FramebufferSizeCallback(GLFWwindow *Window, int Width, int Height);
void MouseScrollCallback(GLFWwindow *Window, double OffsetX, double OffsetY);
void ProcessInput(context *Context);

// settings
const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;

static context Context;

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
    Context = {
        .Window = Window,
        .ScreenWidth = (float)SCREEN_WIDTH,
        .ScreenHeight = (float)SCREEN_HEIGHT,
        .DeltaTime = 0.0f,
        .LastX = SCREEN_WIDTH / 2.0f,
        .LastY = SCREEN_HEIGHT / 2.0f,
        .FirstClick = true,
    };

    if (Context.Window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Context.Window);
    glfwSetFramebufferSizeCallback(Context.Window, FramebufferSizeCallback);
    glfwSetScrollCallback(Context.Window, MouseScrollCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    gui Gui = GuiCreate(&Context);
    renderer Renderer = RendererCreate();
    camera Camera = CameraCreate(glm::vec3(0.0f, 0.0f, 3.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);

    entity Entity = {
        .Position = glm::vec3(0.0f),
        .Scale = glm::vec3(0.0f),
        .Rotation = glm::vec3(0.0f),
        .Color = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f),
    };
    light_entity LightEntity = {
        .Entity =
            {
                .Position = glm::vec3(1.2f, 1.0f, 2.0f),
                .Scale = glm::vec3(0.0f),
                .Rotation = glm::vec3(0.0f),
                .Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
            },
        .AmbientStrength = 0.1f,
        .SpecularStrength = 0.5,
    };
    Context.Entity = &Entity;
    Context.LightEntity = &LightEntity;
    Context.Camera = &Camera;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(Context.Window)) {
        // per-frame time logic
        // --------------------
        float CurrentFrame = static_cast<float>(glfwGetTime());
        Context.DeltaTime = CurrentFrame - Context.LastFrame;
        Context.LastFrame = CurrentFrame;

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

        BeginMode3D(&Renderer, Context.Camera, Context.ScreenWidth,
                    Context.ScreenHeight);

        DrawLight(&Renderer, LightEntity.Position, LightEntity.Color,
                  LightEntity.AmbientStrength, LightEntity.SpecularStrength);
        DrawCube(&Renderer, Entity.Position, Entity.Color);

        // TODO: End the camera 3d rendering
        // EndMode3D();

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

    if (glfwGetKey(Context->Window, GLFW_KEY_W) == GLFW_PRESS) {
        CameraProcessKeyboard(Context->Camera, FORWARD, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_S) == GLFW_PRESS) {
        CameraProcessKeyboard(Context->Camera, BACKWARD, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_A) == GLFW_PRESS) {
        CameraProcessKeyboard(Context->Camera, LEFT, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_D) == GLFW_PRESS) {
        CameraProcessKeyboard(Context->Camera, RIGHT, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        CameraProcessKeyboard(Context->Camera, UP, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        CameraProcessKeyboard(Context->Camera, DOWN, Context->DeltaTime);
    }

    if (glfwGetMouseButton(Context->Window, GLFW_MOUSE_BUTTON_LEFT) ==
        GLFW_PRESS) {
        // INFO: Hide the mouse cursor on left mouse button click
        glfwSetInputMode(Context->Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (Context->FirstClick) {
            // Center the cursor and reset the last mouse position
            glfwSetCursorPos(Context->Window, (Context->ScreenWidth / 2.0f),
                             (Context->ScreenHeight / 2.0f));
            Context->LastX = Context->ScreenWidth / 2.0f;
            Context->LastY = Context->ScreenHeight / 2.0f;
            Context->FirstClick = false;
        }

        double MouseX;
        double MouseY;
        glfwGetCursorPos(Context->Window, &MouseX, &MouseY);

        float OffsetX = (float)(MouseX)-Context->LastX;
        float OffsetY = Context->LastY - (float)(MouseY);

        Context->LastX = MouseX;
        Context->LastY = MouseY;

        CameraProcessMouseMovement(Context->Camera, OffsetX, OffsetY, true);

        // Reset the mouse position and the last mouse position
        // to calculate the next frame's delta
        glfwSetCursorPos(Context->Window, (Context->ScreenWidth / 2.0f),
                         (Context->ScreenHeight / 2.0f));
        Context->LastX = Context->ScreenWidth / 2.0f;
        Context->LastY = Context->ScreenHeight / 2.0f;
    } else if (glfwGetMouseButton(Context->Window, GLFW_MOUSE_BUTTON_LEFT) ==
               GLFW_RELEASE) {
        glfwSetInputMode(Context->Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        Context->FirstClick = true;
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

void MouseScrollCallback(GLFWwindow *Window, double OffsetX, double OffsetY) {
    CameraProcessMouseScroll(Context.Camera, static_cast<float>(OffsetY));
}
