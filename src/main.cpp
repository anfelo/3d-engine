#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "camera.h"
#include "context.h"
#include "entity.h"
#include "gui.h"
#include "mesh.h"
#include "model.h"
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
        .ScreenWidth = SCREEN_WIDTH,
        .ScreenHeight = SCREEN_HEIGHT,
        .FramebufferWidth = SCREEN_WIDTH,
        .FramebufferHeight = SCREEN_HEIGHT,
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

    glfwGetFramebufferSize(Context.Window, &Context.FramebufferWidth,
                           &Context.FramebufferHeight);
    Context.ScreenWidth = Context.FramebufferWidth;
    Context.ScreenHeight = Context.FramebufferHeight;
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);

    gui Gui = Gui_Create(Context);
    renderer Renderer =
        Renderer_Create(Context.ScreenWidth, Context.ScreenHeight);
    camera Camera = Camera_Create(glm::vec3(0.0f, 0.0f, 3.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);
    Context.Camera = Camera;

    scene Scene = Scene_Create();

    texture SkyboxCubemap;
    std::vector<std::string> SkyboxFaces{
        "./resources/textures/skybox/right.jpg",
        "./resources/textures/skybox/left.jpg",
        "./resources/textures/skybox/top.jpg",
        "./resources/textures/skybox/bottom.jpg",
        "./resources/textures/skybox/front.jpg",
        "./resources/textures/skybox/back.jpg",
    };
    Texture_CreateCubemap(&SkyboxCubemap, SkyboxFaces);
    std::vector<texture> SkyboxTextures = {SkyboxCubemap};

    mesh SkyboxMesh;
    Mesh_CreateCube(&SkyboxMesh, SkyboxTextures);

    skybox Skybox = {.Mesh = SkyboxMesh};
    Scene.Skybox = Skybox;

    texture ContainerDiffuseMap;
    Texture_Create(&ContainerDiffuseMap, "./resources/textures/container.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    ContainerDiffuseMap.Name = "diffuse";
    texture ContainerSpecularMap;
    Texture_Create(&ContainerSpecularMap,
                   "./resources/textures/container_specular.png", GL_TEXTURE_2D,
                   GL_TEXTURE1, GL_RGBA, GL_UNSIGNED_BYTE);
    ContainerSpecularMap.Name = "specular";

    std::vector<texture> ContainerTextures = {ContainerDiffuseMap,
                                              ContainerSpecularMap};

    mesh ContainerMesh;
    Mesh_CreateCube(&ContainerMesh, ContainerTextures);

    entity Container = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(0.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(0.0f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .Color = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f),
        .IsSelected = false,
        .Mesh = ContainerMesh,
    };

    Scene_AddEntity(Scene, Container);

    texture RockDiffuseMap;
    Texture_Create(&RockDiffuseMap,
                   "./resources/textures/dry_riverbed_rock_diff.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    RockDiffuseMap.Name = "diffuse";
    texture RockNormalMap;
    Texture_Create(&RockNormalMap,
                   "./resources/textures/dry_riverbed_rock_normal.png",
                   GL_TEXTURE_2D, GL_TEXTURE2, GL_RGBA, GL_UNSIGNED_BYTE);
    RockNormalMap.Name = "normal";

    std::vector<texture> RockTextures = {RockDiffuseMap, RockNormalMap};

    mesh RockMesh = ContainerMesh;
    RockMesh.Textures = RockTextures;

    entity Rock = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(2.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(0.0f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .Color = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f),
        .IsSelected = false,
        .Mesh = RockMesh,
    };

    Scene_AddEntity(Scene, Rock);

    model BackpackModel;
    Model_Create(&BackpackModel, "./resources/models/backpack/backpack.obj",
                 false);

    entity Backpack = {
        .Type = entity_type::Model,
        .Position = glm::vec3(4.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(0.4f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .Color = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f),
        .IsSelected = false,
        .Model = BackpackModel,
    };

    Scene_AddEntity(Scene, Backpack);

    texture GrassTexture;
    Texture_Create(&GrassTexture, "./resources/textures/grass.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

    std::vector<texture> GrassTextures = {GrassTexture};

    std::vector<glm::vec3> Vegetation;
    Vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    Vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    Vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    Vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    Vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    mesh GrassMesh;
    Mesh_CreateQuad(&GrassMesh, GrassTextures);

    for (unsigned int i = 0; i < Vegetation.size(); i++) {
        entity Grass = {
            .Type = entity_type::QuadMesh,
            .Position = Vegetation[i],
            .Scale = glm::vec3(1.0f),
            .Rotation = glm::vec4(0.0f),
            .Color = glm::vec4(1.0f),
            .Mesh = GrassMesh,
        };

        Scene_AddEntity(Scene, Grass);
    }

    texture WindowTexture;
    Texture_Create(&WindowTexture,
                   "./resources/textures/blending_transparent_window.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

    std::vector<texture> WindowTextures = {WindowTexture};

    std::vector<glm::vec3> Windows;
    Windows.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    Windows.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    Windows.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    Windows.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    Windows.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    glm::vec3 CameraPosition = Camera.Position;
    sort(Windows.begin(), Windows.end(),
         [&CameraPosition](const glm::vec3 &A, const glm::vec3 &B) {
             return length(CameraPosition - A) >= length(CameraPosition - B);
         });

    mesh WindowMesh;
    Mesh_CreateQuad(&WindowMesh, WindowTextures);

    for (unsigned int i = 0; i < Windows.size(); i++) {
        entity Window = {
            .Type = entity_type::QuadMesh,
            .Position = Windows[i],
            .Scale = glm::vec3(1.0f),
            .Rotation = glm::vec4(0.0f),
            .Color = glm::vec4(1.0f),
            .Mesh = WindowMesh,
        };

        Scene_AddEntity(Scene, Window);
    }

    // Lights
    glm::vec3 PointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f),
    };
    size_t PointLightPositionsLength =
        sizeof(PointLightPositions) / sizeof(PointLightPositions[0]);

    for (size_t i = 0; i < PointLightPositionsLength; i++) {
        light PointLight = {
            .Entity =
                {
                    .Position = PointLightPositions[i],
                    .Scale = glm::vec3(1.0f),
                    .Rotation = glm::vec4(0.0f),
                    .Color = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f),
                },
            .LightType = light_type::Point,
            .AmbientStrength = 0.1f,
            .SpecularStrength = 0.5,
        };
        Scene_AddLight(Scene, PointLight);
    }

    light SpotLight = {
        .Entity =
            {
                .Position = Camera.Position,
                .Scale = glm::vec3(1.0f),
                .Rotation = glm::vec4(0.0f),
                .Color = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f),
            },
        .LightType = light_type::Spot,
        .AmbientStrength = 0.1f,
        .SpecularStrength = 0.5,
    };
    Scene_AddLight(Scene, SpotLight);

    light DirectionalLight = {
        .Entity =
            {
                .Position = glm::vec3(0.0f),
                .Scale = glm::vec3(1.0f),
                .Rotation = glm::vec4(0.0f),
                .Color = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f),
            },
        .LightType = light_type::Directional,
        .AmbientStrength = 0.1f,
        .SpecularStrength = 0.5,
    };
    Scene_AddLight(Scene, DirectionalLight);

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
        Renderer_ClearBackground(0.1f, 0.1f, 0.1f, 1.0f);

        Renderer_DrawScene(Renderer, Scene, Context);

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

    Context.ScreenWidth = Width;
    Context.ScreenHeight = Height;
}

void MouseScrollCallback(GLFWwindow *Window, double OffsetX, double OffsetY) {
    Camera_ProcessMouseScroll(&Context.Camera, static_cast<float>(OffsetY));
}
