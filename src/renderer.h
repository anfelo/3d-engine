#ifndef RENDERER_H_
#define RENDERER_H_

#include "camera.h"

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

struct rectangle_mesh {
    float Vertices[12];
    GLuint Indices[6];

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

struct cube_mesh {
    float Vertices[216];

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
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
};

struct shader_program {
    GLuint ID;

    uniform_locators Uniforms;
};

struct renderer {
    shader_program ShaderProgram;
    shader_program OutlineShaderProgram;

    triangle_mesh TriangleMesh;
    rectangle_mesh RectangleMesh;
    cube_mesh CubeMesh;
};

renderer RendererCreate(void);
void RendererDestroy(renderer *Renderer);
void ShaderCreate(shader_program *Shader, const char *VertexFile,
                  const char *FragmentFile);
void DrawTriangle(renderer *Renderer, glm::vec<3, float> position);
void DrawRectangle(renderer *Renderer, glm::vec<3, float> position);
void DrawCube(renderer *Renderer, glm::vec<3, float> Position,
              glm::vec<4, float> Color, bool IsSelected);
void DrawLight(renderer *Renderer, glm::vec<3, float> Position,
               glm::vec<4, float> Color, float AmbientStrength,
               float SpecularStrength);
void ClearBackground(float R, float G, float B, float Alpha);
void BeginMode3D(renderer *Renderer, camera *Camera, float ScreenWidth,
                 float ScreenHeight);

#endif
