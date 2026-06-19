#ifndef SHADER_H_
#define SHADER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

struct shader {
    GLuint ID;
    std::unordered_map<std::string, GLuint> Uniforms;
};

void Shader_Create(shader &Shader, const char *VertexFile,
                   const char *FragmentFile,
                   const char *GeometryFile = nullptr);
void Shader_Delete(shader &Shader);
void Shader_Use(const shader &Shader);
GLuint Shader_GetUniform(const shader &Shader, const char *Name);

void Shader_SetMat4(const shader &Shader, const char *Name,
                    const glm::mat4 &Value);
void Shader_SetVec3(const shader &Shader, const char *Name,
                    const glm::vec3 &Value);
void Shader_SetVec2(const shader &Shader, const char *Name,
                    const glm::vec2 &Value);
void Shader_SetVec4(const shader &Shader, const char *Name,
                    const glm::vec4 &Value);
void Shader_SetFloat(const shader &Shader, const char *Name, float Value);
void Shader_SetInt(const shader &Shader, const char *Name, int Value);

#endif
