#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

std::string GetFileContents(const char *Filename) {
    std::ifstream in(Filename, std::ios::binary);
    if (in) {
        std::string Contents;
        in.seekg(0, std::ios::end);
        Contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&Contents[0], Contents.size());
        in.close();
        return Contents;
    }
    throw errno;
}

void Shader_Create(shader &Shader, const char *VertexFile,
                   const char *FragmentFile, const char *GeometryFile) {
    std::string VertexCode;
    std::string FragmentCode;

    VertexCode = GetFileContents(VertexFile);
    FragmentCode = GetFileContents(FragmentFile);

    const char *VertexShaderSource = VertexCode.c_str();
    const char *FragmentShaderSource = FragmentCode.c_str();

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);

    // check for shader compile errors
    int Success;
    char InfoLog[512];
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
    if (!Success) {
        glGetShaderInfoLog(VertexShader, 512, NULL, InfoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << InfoLog << std::endl;
    }

    // fragment shader
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
    glCompileShader(FragmentShader);
    // check for shader compile errors
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
    if (!Success) {
        glGetShaderInfoLog(FragmentShader, 512, NULL, InfoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << InfoLog << std::endl;
    }

    GLuint GeometryShader;
    if (GeometryFile) {
        std::string GeometryCode = GetFileContents(GeometryFile);
        const char *GeometryShaderSource = GeometryCode.c_str();

        GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(GeometryShader, 1, &GeometryShaderSource, NULL);
        glCompileShader(GeometryShader);
        // check for shader compile errors
        glGetShaderiv(GeometryShader, GL_COMPILE_STATUS, &Success);
        if (!Success) {
            glGetShaderInfoLog(GeometryShader, 512, NULL, InfoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << InfoLog << std::endl;
        }
    }

    // link shaders
    Shader.ID = glCreateProgram();
    glAttachShader(Shader.ID, VertexShader);
    glAttachShader(Shader.ID, FragmentShader);
    if (GeometryFile) {
        glAttachShader(Shader.ID, GeometryShader);
    }

    glLinkProgram(Shader.ID);
    // check for linking errors
    glGetProgramiv(Shader.ID, GL_LINK_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(Shader.ID, 512, NULL, InfoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << InfoLog << std::endl;
    }
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    if (GeometryFile) {
        glDeleteShader(GeometryShader);
    }
}

void Shader_Use(const shader &Shader) {
    glUseProgram(Shader.ID);
}

void Shader_Delete(shader &Shader) {
    glDeleteProgram(Shader.ID);
}

GLuint Shader_GetUniform(const shader &Shader, const char *Name) {
    return glGetUniformLocation(Shader.ID, Name);
}

void Shader_SetFloat(const shader &Shader, const char *Name, float Value) {
    Shader_Use(Shader);
    glUniform1f(Shader_GetUniform(Shader, Name), Value);
}

void Shader_SetInt(const shader &Shader, const char *Name, int Value) {
    Shader_Use(Shader);
    glUniform1i(Shader_GetUniform(Shader, Name), Value);
}

void Shader_SetMat4(const shader &Shader, const char *Name,
                    const glm::mat4 &Value) {
    Shader_Use(Shader);
    glUniformMatrix4fv(Shader_GetUniform(Shader, Name), 1, GL_FALSE,
                       glm::value_ptr(Value));
}

void Shader_SetVec2(const shader &Shader, const char *Name,
                    const glm::vec2 &Value) {
    Shader_Use(Shader);
    glUniform2fv(Shader_GetUniform(Shader, Name), 1, glm::value_ptr(Value));
}

void Shader_SetVec3(const shader &Shader, const char *Name,
                    const glm::vec3 &Value) {
    Shader_Use(Shader);
    glUniform3fv(Shader_GetUniform(Shader, Name), 1, glm::value_ptr(Value));
}

void Shader_SetVec4(const shader &Shader, const char *Name,
                    const glm::vec4 &Value) {
    Shader_Use(Shader);
    glUniform4fv(Shader_GetUniform(Shader, Name), 1, glm::value_ptr(Value));
}

const char *ToString(shader_type ShaderType) {
    switch (ShaderType) {
    case shader_type::Lit:
        return "Lit";
    case shader_type::Unlit:
        return "Unlit";
    case shader_type::Water:
        return "Water";
    case shader_type::Outline:
        return "Outline";
    case shader_type::Skybox:
        return "Skybox";
    case shader_type::Screen:
        return "Screen";
    case shader_type::Blur:
        return "Blur";
    case shader_type::Depth:
        return "Depth";
    case shader_type::CubemapDepth:
        return "CubemapDepth";
    case shader_type::Gui:
        return "Gui";
    case shader_type::Quad:
        return "Quad";
    case shader_type::Instance:
        return "Instance";
        break;
    }
    return "Unknown";
}
