#include "renderer.h"

#include <string>
#include <fstream>
#include <iostream>
#include <cerrno>

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

renderer RendererCreate() {
    renderer Renderer;
    Renderer = {
        .TriangleMesh{
            .Vertices{-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f},
        },
        .RectangleMesh{.Vertices{
                           0.5f, 0.5f, 0.0f,   // top right
                           0.5f, -0.5f, 0.0f,  // bottom right
                           -0.5f, -0.5f, 0.0f, // bottom left
                           -0.5f, 0.5f, 0.0f   // top left
                       },
                       .Indices{0, 1, 3, 1, 2, 3}}};

    std::string vertex_code =
        GetFileContents("./resources/shaders/default.vert");
    std::string fragment_code =
        GetFileContents("resources/shaders/default.frag");

    const char *VertexShaderSource = vertex_code.c_str();
    const char *FragmentShaderSource = fragment_code.c_str();

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

    // link shaders
    Renderer.ShaderProgram.ID = glCreateProgram();
    glAttachShader(Renderer.ShaderProgram.ID, VertexShader);
    glAttachShader(Renderer.ShaderProgram.ID, FragmentShader);
    glLinkProgram(Renderer.ShaderProgram.ID);
    // check for linking errors
    glGetProgramiv(Renderer.ShaderProgram.ID, GL_LINK_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(Renderer.ShaderProgram.ID, 512, NULL, InfoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << InfoLog << std::endl;
    }
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    // ### Triangle ###
    glGenBuffers(1, &Renderer.TriangleMesh.VBO);
    glGenVertexArrays(1, &Renderer.TriangleMesh.VAO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(Renderer.TriangleMesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer.TriangleMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer.TriangleMesh.Vertices),
                 Renderer.TriangleMesh.Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ### Rectangle ###
    glGenBuffers(1, &Renderer.RectangleMesh.VBO);
    glGenVertexArrays(1, &Renderer.RectangleMesh.VAO);
    glGenBuffers(1, &Renderer.RectangleMesh.EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(Renderer.RectangleMesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer.RectangleMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer.RectangleMesh.Vertices),
                 Renderer.RectangleMesh.Vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer.RectangleMesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(Renderer.RectangleMesh.Indices),
                 Renderer.RectangleMesh.Indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return Renderer;
}

void RendererDestroy(renderer *Renderer) {
    glDeleteVertexArrays(1, &Renderer->TriangleMesh.VAO);
    glDeleteBuffers(1, &Renderer->TriangleMesh.VBO);

    glDeleteVertexArrays(1, &Renderer->RectangleMesh.VAO);
    glDeleteBuffers(1, &Renderer->RectangleMesh.VBO);
    glDeleteBuffers(1, &Renderer->RectangleMesh.EBO);

    glDeleteProgram(Renderer->ShaderProgram.ID);
}

void ClearBackground(float R, float G, float B, float Alpha) {
    glClearColor(R, G, B, Alpha);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DrawTriangle(renderer *Renderer) {
    glUseProgram(Renderer->ShaderProgram.ID);
    glBindVertexArray(Renderer->TriangleMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void DrawRectangle(renderer *Renderer) {
    glUseProgram(Renderer->ShaderProgram.ID);
    glBindVertexArray(Renderer->RectangleMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
