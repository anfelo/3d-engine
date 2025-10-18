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

void RendererInit(shader_program *ShaderProgram, mesh *Mesh) {
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
    ShaderProgram->ID = glCreateProgram();
    glAttachShader(ShaderProgram->ID, VertexShader);
    glAttachShader(ShaderProgram->ID, FragmentShader);
    glLinkProgram(ShaderProgram->ID);
    // check for linking errors
    glGetProgramiv(ShaderProgram->ID, GL_LINK_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram->ID, 512, NULL, InfoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << InfoLog << std::endl;
    }
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // We specify that we only have 1 3D object
    glGenBuffers(1, &Mesh->VBO);
    glGenVertexArrays(1, &Mesh->VAO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(Mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh->Vertices), Mesh->Vertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
}

void RendererDestroy(shader_program *ShaderProgram, mesh *Mesh) {
    glDeleteVertexArrays(1, &Mesh->VAO);
    glDeleteBuffers(1, &Mesh->VBO);
    glDeleteProgram(ShaderProgram->ID);
}

void DrawTriangle(shader_program *ShaderProgram, mesh *Mesh) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ShaderProgram->ID);
    glBindVertexArray(Mesh->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
