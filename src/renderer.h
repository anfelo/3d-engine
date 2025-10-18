#ifndef RENDERER_H_
#define RENDERER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

struct shader_program {
    GLuint ID;
};

struct mesh {
    float Vertices[9];

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

struct renderer {
    shader_program ShaderProgram;
    mesh Mesh;
};

renderer RendererCreate(void);
void RendererDestroy(renderer *Renderer);
void DrawTriangle(renderer *Renderer);
void ClearBackground(float R, float G, float B, float Alpha);

#endif
