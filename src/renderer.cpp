#include "renderer.h"

const char *VertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *FragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

renderer RendererCreate() {
    renderer Renderer;
    Renderer = {.Mesh{
        .Vertices{-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f}}};

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

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // We specify that we only have 1 3D object
    glGenBuffers(1, &Renderer.Mesh.VBO);
    glGenVertexArrays(1, &Renderer.Mesh.VAO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(Renderer.Mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer.Mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer.Mesh.Vertices),
                 Renderer.Mesh.Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    return Renderer;
}

void RendererDestroy(renderer *Renderer) {
    glDeleteVertexArrays(1, &Renderer->Mesh.VAO);
    glDeleteBuffers(1, &Renderer->Mesh.VBO);
    glDeleteProgram(Renderer->ShaderProgram.ID);
}

void ClearBackground(float R, float G, float B, float Alpha) {
    glClearColor(R, G, B, Alpha);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DrawTriangle(renderer *Renderer) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(Renderer->ShaderProgram.ID);
    glBindVertexArray(Renderer->Mesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
