#ifndef RENDERER_H_
#define RENDERER_H_

#include "camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

struct shader_program {
    GLuint ID;
};

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
    float Vertices[108];

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
};

struct renderer {
    shader_program ShaderProgram;

    triangle_mesh TriangleMesh;
    rectangle_mesh RectangleMesh;
    cube_mesh CubeMesh;

    uniform_locators Uniforms;
};

renderer RendererCreate(void);
void RendererDestroy(renderer *Renderer);
void DrawTriangle(renderer *Renderer, glm::vec<3, float> position);
void DrawRectangle(renderer *Renderer, glm::vec<3, float> position);
void DrawCube(renderer *Renderer, glm::vec<3, float> Position,
              glm::vec<4, float> Color);
void DrawLight(renderer *Renderer, glm::vec<3, float> Position,
               glm::vec<4, float> Color);
void ClearBackground(float R, float G, float B, float Alpha);
void BeginMode3D(renderer *Renderer, camera *Camera, float ScreenWidth,
                 float ScreenHeight);

#endif
