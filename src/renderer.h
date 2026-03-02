#ifndef RENDERER_H_
#define RENDERER_H_

#include "camera.h"
#include "context.h"
#include "scene.h"
#include "texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

struct triangle_mesh {
    float Vertices[9];

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

struct quad_mesh {
    float Vertices[12];
    GLuint Indices[6];

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

struct cube_mesh {
    float Vertices[288];

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

struct directional_light {
    GLuint DirUniformLoc;
    GLuint AmbientUniformLoc;
    GLuint DiffuseUniformLoc;
    GLuint SpecularUniformLoc;
};

struct point_light {
    GLuint PositionUniformLoc;
    GLuint AmbientUniformLoc;
    GLuint DiffuseUniformLoc;
    GLuint SpecularUniformLoc;
    GLuint ConstantUniformLoc;
    GLuint LinearUniformLoc;
    GLuint QuadraticUniformLoc;
};

struct spot_light {
    GLuint PositionUniformLoc;
    GLuint DirectionUniformLoc;
    GLuint AmbientUniformLoc;
    GLuint DiffuseUniformLoc;
    GLuint SpecularUniformLoc;
    GLuint ConstantUniformLoc;
    GLuint LinearUniformLoc;
    GLuint QuadraticUniformLoc;
    GLuint CutOffUniformLoc;
    GLuint OuterCutOffUniformLoc;
};

struct material_uniforms {
    GLuint DiffuseUniformLoc;
    GLuint SpecularUniformLoc;
    GLuint NormalUniformLoc;
    GLuint ShininessUniformLoc;
    GLuint HasNormalUniformLoc;
    GLuint HasSpecularUniformLoc;
};

struct uniform_locators {
    GLuint ModelUniformLoc;
    GLuint ViewUniformLoc;
    GLuint ProjectionUniformLoc;

    GLuint EntityColorUniformLoc;
    GLuint LightColorUniformLoc;
    GLuint LightPositionUniformLoc;
    GLuint ViewPositionUniformLoc;
    GLuint AmbientStrengthUniformLoc;
    GLuint SpecularStrengthUniformLoc;

    GLuint ScreenTextureUniformLoc;

    material_uniforms Material;

    directional_light DirectionalLight;
    spot_light SpotLight;
    point_light PointLights[4];
};

struct shader_program {
    GLuint ID;

    uniform_locators Uniforms;
};

struct renderer {
    shader_program ShaderProgram;
    shader_program OutlineShaderProgram;
    shader_program QuadShaderProgram;
    shader_program ScreenShaderProgram;
    shader_program SkyBoxShaderProgram;
    shader_program InstanceShaderProgram;

    // Framebuffer stuff
    GLuint FrameBufferVAO, FrameBufferVBO;
    GLuint FrameBuffer;
    GLuint TextureColorBuffer;
    GLuint RBO; // render buffer object
};

renderer Renderer_Create(int ScreenWidth, int ScreenHeight);
void Renderer_Destroy(renderer &Renderer);
void Renderer_ResizeFramebuffer(const renderer &Renderer, int ScreenWidth,
                                int ScreenHeight);
shader_program Renderer_CreateShaderProgram(const char *VertexFile,
                                            const char *FragmentFile);
void Renderer_DrawTriangle(const renderer &Renderer,
                           glm::vec<3, float> position);
void Renderer_DrawQuad(const renderer &Renderer, glm::vec<3, float> Position,
                       material Material);
void Renderer_DrawQuadMesh(const renderer &Renderer,
                           glm::vec<3, float> Position, mesh Mesh);
void Renderer_DrawCube(const renderer &Renderer, glm::vec<3, float> Position,
                       glm::vec<4, float> Rotation, glm::vec<4, float> Color,
                       material Material, bool IsSelected);
void Renderer_DrawCubeMesh(const renderer &Renderer,
                           glm::vec<3, float> Position,
                           glm::vec<4, float> Rotation,
                           glm::vec<4, float> Color, mesh CubeMesh,
                           bool IsSelected);
void Renderer_DrawModel(const renderer &Renderer, glm::vec<3, float> Position,
                        glm::vec<3, float> Scale, glm::vec<4, float> Rotation,
                        glm::vec<4, float> Color, model EntityModel,
                        bool IsSelected);
void Renderer_DrawLight(const renderer &Renderer, glm::vec<3, float> Position,
                        glm::vec<4, float> Color, float AmbientStrength,
                        float SpecularStrength);
void Renderer_DrawSkybox(const renderer &Renderer, const skybox &Skybox);
void Renderer_DrawScene(const renderer &Renderer, const scene &Scene,
                        const context &Context);
void Renderer_DrawSceneLights(const renderer &Renderer,
                              shader_program ShaderProgram, const scene &Scene,
                              const camera &Camera);
void Renderer_ClearBackground(float R, float G, float B, float Alpha);
void Renderer_SetCameraUniforms(const renderer &Renderer, const camera &Camera,
                                float ScreenWidth, float ScreenHeight);

triangle_mesh Renderer_GetTriangleMesh();
quad_mesh Renderer_GetQuadMesh();
cube_mesh Renderer_GetCubeMesh();

#endif
