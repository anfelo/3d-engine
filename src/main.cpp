#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "camera.h"
#include "context.h"
#include "entity.h"
#include "gui.h"
#include "renderer.h"
#include "scene.h"

void FramebufferSizeCallback(GLFWwindow *Window, int Width, int Height);
void MouseScrollCallback(GLFWwindow *Window, double OffsetX, double OffsetY);
void ProcessInput(context *Context);

// settings
const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;

context Context;

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

    gui Gui = Gui_Create(Context);
    renderer Renderer = Renderer_Create();
    camera Camera = Camera_Create(glm::vec3(0.0f, 0.0f, 3.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);
    Context.Camera = Camera;

    scene Scene = Scene_Create();

    glm::vec3 CubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f),
    };

    // glm::vec3 PointLightPositions[] = {
    //     glm::vec3(0.7f, 0.2f, 2.0f),
    //     glm::vec3(2.3f, -3.3f, -4.0f),
    //     glm::vec3(-4.0f, 2.0f, -12.0f),
    //     glm::vec3(0.0f, 0.0f, -3.0f),
    // };

    for (size_t i = 0; i < 10; i++) {
        float Angle = 20 * i;
        entity Cube = {
            .Type = entity_type::Cube,
            .Position = CubePositions[i],
            .Scale = glm::vec3(0.0f),
            .Rotation = glm::vec4(Angle, 1.0f, 0.3f, 0.5f),
            .Color = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f),
            .IsSelected = false,
        };

        Scene_AddEntity(Scene, Cube);
    }

    light_entity LightEntity = {
        .Entity =
            {
                .Position = glm::vec3(1.2f, 1.0f, 2.0f),
                .Scale = glm::vec3(1.0f),
                .Rotation = glm::vec4(0.0f),
                .Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
            },
        .AmbientStrength = 0.1f,
        .SpecularStrength = 0.5,
    };

    Scene_AddLight(Scene, LightEntity);

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
        Gui_NewFrame();

        // render
        // ------
        Renderer_ClearBackground(0.1f, 0.1f, 0.1f, 1.0f);

        Renderer_BeginMode3D(Renderer, Context.Camera, Context.ScreenWidth,
                             Context.ScreenHeight);

        Renderer_DrawScene(Renderer, Scene, Context.Camera);

        // TODO: End the camera 3d rendering
        // Renderer_EndMode3D();

        // gui
        // ------
        Gui_Draw(Scene);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(Context.Window);
        glfwPollEvents();
    }

    Gui_Destroy();
    Renderer_Destroy(Renderer);

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
        Camera_ProcessKeyboard(&Context->Camera, FORWARD, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_S) == GLFW_PRESS) {
        Camera_ProcessKeyboard(&Context->Camera, BACKWARD, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_A) == GLFW_PRESS) {
        Camera_ProcessKeyboard(&Context->Camera, LEFT, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_D) == GLFW_PRESS) {
        Camera_ProcessKeyboard(&Context->Camera, RIGHT, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        Camera_ProcessKeyboard(&Context->Camera, UP, Context->DeltaTime);
    }

    if (glfwGetKey(Context->Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        Camera_ProcessKeyboard(&Context->Camera, DOWN, Context->DeltaTime);
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

        Camera_ProcessMouseMovement(&Context->Camera, OffsetX, OffsetY, true);

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
    Camera_ProcessMouseScroll(&Context.Camera, static_cast<float>(OffsetY));
}
