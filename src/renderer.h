#ifndef RENDERER_H_
#define RENDERER_H_

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

struct renderer {
    shader_program ShaderProgram;

    triangle_mesh TriangleMesh;
    rectangle_mesh RectangleMesh;
};

renderer RendererCreate(void);
void RendererDestroy(renderer *Renderer);
void DrawTriangle(renderer *Renderer);
void DrawRectangle(renderer *Renderer);
void ClearBackground(float R, float G, float B, float Alpha);

#endif
