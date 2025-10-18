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

void RendererInit(shader_program_t *shaderProgram, mesh_t *mesh) {
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // link shaders
    shaderProgram->ID = glCreateProgram();
    glAttachShader(shaderProgram->ID, vertexShader);
    glAttachShader(shaderProgram->ID, fragmentShader);
    glLinkProgram(shaderProgram->ID);
    // check for linking errors
    glGetProgramiv(shaderProgram->ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram->ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // We specify that we only have 1 3D object
    glGenBuffers(1, &mesh->VBO);
    glGenVertexArrays(1, &mesh->VAO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->vertices), mesh->vertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
}

void RendererDestroy(shader_program_t *shaderProgram, mesh_t *mesh) {
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteProgram(shaderProgram->ID);
}

void DrawTriangle(shader_program_t *shaderProgram, mesh_t *mesh) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram->ID);
    glBindVertexArray(mesh->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
