#ifndef RENDERER_H_
#define RENDERER_H_

#include "camera.h"
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
    GLuint ShininessUniformLoc;
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
};

renderer Renderer_Create(void);
void Renderer_Destroy(renderer &Renderer);
shader_program Renderer_CreateShaderProgram(const char *VertexFile,
                                            const char *FragmentFile);
void Renderer_DrawTriangle(const renderer &Renderer,
                           glm::vec<3, float> position);
void Renderer_DrawQuad(const renderer &Renderer, glm::vec<3, float> position);
void Renderer_DrawCube(const renderer &Renderer, glm::vec<3, float> Position,
                       glm::vec<4, float> Rotation, glm::vec<4, float> Color,
                       material Material, bool IsSelected);
void Renderer_DrawLight(const renderer &Renderer, glm::vec<3, float> Position,
                        glm::vec<4, float> Color, float AmbientStrength,
                        float SpecularStrength);
void Renderer_DrawScene(const renderer &Renderer, const scene &Scene,
                        const camera &Camera);
void Renderer_ClearBackground(float R, float G, float B, float Alpha);
void Renderer_BeginMode3D(const renderer &Renderer, const camera &Camera,
                          float ScreenWidth, float ScreenHeight);

triangle_mesh Renderer_GetTriangleMesh();
quad_mesh Renderer_GetQuadMesh();
cube_mesh Renderer_GetCubeMesh();

#endif
