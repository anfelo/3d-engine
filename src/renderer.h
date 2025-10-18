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

void RendererInit(shader_program *ShaderProgram, mesh *Mesh);
void RendererDestroy(shader_program *ShaderProgram, mesh *Mesh);
void DrawTriangle(shader_program *ShaderProgram, mesh *Mesh);

#endif
