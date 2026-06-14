#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "camera.h"
#include "context.h"
#include "gui.h"
#include "renderer.h"
#include "resource_manager.h"
#include "scene.h"

void FramebufferSizeCallback(GLFWwindow *Window, int Width, int Height);
void MouseScrollCallback(GLFWwindow *Window, double OffsetX, double OffsetY);
void ProcessInput(context *Context);

// settings
const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;

context Context = {0};

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
        .ScreenWidth = SCREEN_WIDTH,
        .ScreenHeight = SCREEN_HEIGHT,
        .FramebufferWidth = SCREEN_WIDTH,
        .FramebufferHeight = SCREEN_HEIGHT,
        .ShadowbufferWidth = 1024,
        .ShadowbufferHeight = 1024,
        .DeltaTime = 0.0f,
        .CurrentSceneIdx = 5,
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

    glfwGetFramebufferSize(Context.Window, &Context.FramebufferWidth,
                           &Context.FramebufferHeight);
    Context.ScreenWidth = Context.FramebufferWidth;
    Context.ScreenHeight = Context.FramebufferHeight;
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);

    gui Gui = Gui_Create(Context);
    renderer Renderer = Renderer_Create(Context);
    camera Camera = Camera_Create(glm::vec3(0.0f, 0.0f, 3.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);
    Context.Camera = Camera;

    scene Scene1 = Scene_Create();
    scene Scene2 = Scene_Create();
    scene Scene3 = Scene_Create();
    scene Scene4 = Scene_Create();
    scene Scene5 = Scene_Create();
    scene Scene6 = Scene_Create();

    Context.Scenes.push_back(&Scene1);
    Context.Scenes.push_back(&Scene2);
    Context.Scenes.push_back(&Scene3);
    Context.Scenes.push_back(&Scene4);
    Context.Scenes.push_back(&Scene5);
    Context.Scenes.push_back(&Scene6);

    Scene_BuildScene1(Scene1, Context.Camera);
    Scene_BuildScene2(Scene2, Context.Camera);
    Scene_BuildScene3(Scene3, Context.Camera);
    Scene_BuildScene4(Scene4, Context.Camera);
    Scene_BuildScene5(Scene5, Context.Camera);
    Scene_BuildScene6(Scene6, Context.Camera);

    // texture RefractionGuiTexture = {};
    // RefractionGuiTexture.ID = Renderer.RefractionColorBuffer;
    // RefractionGuiTexture.Type = GL_TEXTURE_2D;
    // RefractionGuiTexture.Name = "diffuse";
    // std::vector<texture> RefractionGuiTextures = {RefractionGuiTexture};
    // material RefractionGuiTextureMaterial = {};
    // Material_Create(RefractionGuiTextureMaterial);
    // RefractionGuiTextureMaterial.Textures = RefractionGuiTextures;
    //
    // texture ReflectionGuiTexture = {};
    // ReflectionGuiTexture.ID = Renderer.ReflectionColorBuffer;
    // ReflectionGuiTexture.Type = GL_TEXTURE_2D;
    // ReflectionGuiTexture.Name = "diffuse";
    // std::vector<texture> ReflectionGuiTextures = {ReflectionGuiTexture};
    // material ReflectionGuiTextureMaterial = {};
    // Material_Create(ReflectionGuiTextureMaterial);
    // ReflectionGuiTextureMaterial.Textures = ReflectionGuiTextures;
    //
    // mesh RefractionGuiTextureMesh;
    // Mesh_CreateGuiQuad(&RefractionGuiTextureMesh,
    // RefractionGuiTextureMaterial);
    //
    // mesh ReflectionGuiTextureMesh;
    // Mesh_CreateGuiQuad(&ReflectionGuiTextureMesh,
    // ReflectionGuiTextureMaterial);
    //
    // entity RefractionGuiEntity = {
    //     .Type = entity_type::QuadMesh,
    //     .Position = glm::vec3(0.0f, 0.0f, 0.0f),
    //     .Scale = glm::vec3(300.0f, 200.0f, 0.0f),
    //     .IsSelected = false,
    //     .Mesh = RefractionGuiTextureMesh,
    // };
    //
    // entity ReflectionGuiEntity = {
    //     .Type = entity_type::QuadMesh,
    //     .Position = glm::vec3(0.0f, 300.0f, 0.0f),
    //     .Scale = glm::vec3(300.0f, 200.0f, 0.0f),
    //     .IsSelected = false,
    //     .Mesh = ReflectionGuiTextureMesh,
    // };
    //
    // Scene_AddGuiTexture(Scene6, RefractionGuiEntity);
    // Scene_AddGuiTexture(Scene6, ReflectionGuiEntity);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(Context.Window)) {
        // per-frame time logic
        // --------------------
        float CurrentFrame = static_cast<float>(glfwGetTime());
        Context.DeltaTime = CurrentFrame - Context.LastFrame;
        Context.LastFrame = CurrentFrame;

        // This keeps the framebuffer dimensions in sync with the window
        // dimensions
        glfwGetFramebufferSize(Context.Window, &Context.FramebufferWidth,
                               &Context.FramebufferHeight);
        if (Context.FramebufferWidth <= 0 || Context.FramebufferHeight <= 0) {
            glfwPollEvents();
            continue;
        }

        if (Context.ScreenWidth != Context.FramebufferWidth ||
            Context.ScreenHeight != Context.FramebufferHeight) {
            Context.ScreenWidth = Context.FramebufferWidth;
            Context.ScreenHeight = Context.FramebufferHeight;
            Renderer_ResizeFramebuffer(Renderer, Context.FramebufferWidth,
                                       Context.FramebufferHeight);
        }

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
        Renderer_ClearBackground(0.01f, 0.01f, 0.01f, 1.0f);

        scene *CurrentScene = Context.Scenes.at(Context.CurrentSceneIdx);
        Renderer_Draw(Renderer, *CurrentScene, Context);

        // gui
        // ------
        Gui_Draw(Context);

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
        int WindowWidth;
        int WindowHeight;
        glfwGetWindowSize(Context->Window, &WindowWidth, &WindowHeight);
        float CenterX = WindowWidth / 2.0f;
        float CenterY = WindowHeight / 2.0f;

        // INFO: Hide the mouse cursor on left mouse button click
        glfwSetInputMode(Context->Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (Context->FirstClick) {
            // Center the cursor and reset the last mouse position
            glfwSetCursorPos(Context->Window, CenterX, CenterY);
            Context->LastX = CenterX;
            Context->LastY = CenterY;
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
        glfwSetCursorPos(Context->Window, CenterX, CenterY);
        Context->LastX = CenterX;
        Context->LastY = CenterY;
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

    Context.ScreenWidth = Width;
    Context.ScreenHeight = Height;
}

void MouseScrollCallback(GLFWwindow *Window, double OffsetX, double OffsetY) {
    Camera_ProcessMouseScroll(&Context.Camera, static_cast<float>(OffsetY));
}
