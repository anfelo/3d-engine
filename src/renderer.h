#ifndef RENDERER_H_
#define RENDERER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

typedef struct shader_program_t {
    GLuint ID;
} shader_program_t;

typedef struct mesh_t {
    float vertices[9];

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
} mesh_t;

void RendererInit(shader_program_t *shaderProgram, mesh_t *mesh);
void RendererDestroy(shader_program_t *shaderProgram, mesh_t *mesh);
void DrawTriangle(shader_program_t *shaderProgram, mesh_t *mesh);

#endif
