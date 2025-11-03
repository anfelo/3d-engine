#include "renderer.h"

#include <string>
#include <fstream>
#include <iostream>
#include <cerrno>

#include "camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

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
                       .Indices{0, 1, 3, 1, 2, 3}},

        .CubeMesh{.Vertices{
            -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f,
            0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
            0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f,
            0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

            -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,
            0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,
            0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f,
            -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
            -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
            1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
            0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,
            1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f,
            0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
            0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,
            0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

            -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
            0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
            0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f}},
    };

    ShaderCreate(&Renderer.ShaderProgram, "./resources/shaders/default.vert",
                 "./resources/shaders/default.frag");
    ShaderCreate(&Renderer.OutlineShaderProgram,
                 "./resources/shaders/default.vert",
                 "./resources/shaders/outline.frag");

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

    // ### Cube ###
    glGenBuffers(1, &Renderer.CubeMesh.VBO);
    glGenVertexArrays(1, &Renderer.CubeMesh.VAO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(Renderer.CubeMesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer.CubeMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer.CubeMesh.Vertices),
                 Renderer.CubeMesh.Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
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

void ShaderCreate(shader_program *Shader, const char *VertexFile,
                  const char *FragmentFile) {
    std::string vertex_code = GetFileContents(VertexFile);
    std::string fragment_code = GetFileContents(FragmentFile);

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
    Shader->ID = glCreateProgram();
    glAttachShader(Shader->ID, VertexShader);
    glAttachShader(Shader->ID, FragmentShader);
    glLinkProgram(Shader->ID);
    // check for linking errors
    glGetProgramiv(Shader->ID, GL_LINK_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(Shader->ID, 512, NULL, InfoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << InfoLog << std::endl;
    }
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    Shader->Uniforms.ModelUniformLoc =
        glGetUniformLocation(Shader->ID, "u_model");
    Shader->Uniforms.ViewUniformLoc =
        glGetUniformLocation(Shader->ID, "u_view");
    Shader->Uniforms.ProjectionUniformLoc =
        glGetUniformLocation(Shader->ID, "u_projection");

    // Fragment Shader Uniform Locators
    // INFO: This are only set on the normal shader
    Shader->Uniforms.EntityColorUniformLoc =
        glGetUniformLocation(Shader->ID, "u_entity_color");
    Shader->Uniforms.LightColorUniformLoc =
        glGetUniformLocation(Shader->ID, "u_light_color");
    Shader->Uniforms.LightPositionUniformLoc =
        glGetUniformLocation(Shader->ID, "u_light_pos");
    Shader->Uniforms.ViewPositionUniformLoc =
        glGetUniformLocation(Shader->ID, "u_view_pos");
    Shader->Uniforms.AmbientStrengthUniformLoc =
        glGetUniformLocation(Shader->ID, "u_ambient_strength");
    Shader->Uniforms.SpecularStrengthUniformLoc =
        glGetUniformLocation(Shader->ID, "u_specular_strength");
}

void ClearBackground(float R, float G, float B, float Alpha) {
    glClearColor(R, G, B, Alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void DrawTriangle(renderer *Renderer, glm::vec<3, float> position) {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection;
    projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Sets the uniform value
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ProjectionUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(model));

    glUseProgram(Renderer->ShaderProgram.ID);
    glBindVertexArray(Renderer->TriangleMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void DrawRectangle(renderer *Renderer, glm::vec<3, float> position) {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection;
    projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Sets the uniform value
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ProjectionUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(model));

    glUseProgram(Renderer->ShaderProgram.ID);
    glBindVertexArray(Renderer->RectangleMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void DrawCube(renderer *Renderer, glm::vec<3, float> Position,
              glm::vec<4, float> Color, bool IsSelected) {
    // 1st render pass
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    glUseProgram(Renderer->ShaderProgram.ID);

    glm::vec3 CubeColor = glm::vec3(Color[0], Color[1], Color[2]);
    glUniform3fv(Renderer->ShaderProgram.Uniforms.EntityColorUniformLoc, 1,
                 glm::value_ptr(CubeColor));

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position);
    Model = glm::scale(Model, glm::vec3(1.0f));
    // TODO: Make the rotation work with the gui params
    float Angle = 20.0f;
    Model = glm::rotate(Model, glm::radians(Angle), glm::vec3(1.0f));
    Model =
        glm::rotate(Model, glm::radians(Angle), glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(Model));

    glBindVertexArray(Renderer->CubeMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    if (IsSelected) {
        // 2st render pass: draws the outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(Renderer->OutlineShaderProgram.ID);

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Position);
        Model = glm::scale(Model, glm::vec3(1.02f));
        // TODO: Make the rotation work with the gui params
        Angle = 20.0f;
        Model = glm::rotate(Model, glm::radians(Angle), glm::vec3(1.0f));
        Model = glm::rotate(Model, glm::radians(Angle),
                            glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(
            Renderer->OutlineShaderProgram.Uniforms.ModelUniformLoc, 1,
            GL_FALSE, glm::value_ptr(Model));

        glBindVertexArray(Renderer->CubeMesh.VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }
}

void DrawLight(renderer *Renderer, glm::vec<3, float> Position,
               glm::vec<4, float> Color, float AmbientStrength,
               float SpecularStrength) {
    glUseProgram(Renderer->ShaderProgram.ID);

    glm::vec3 LightColor = glm::vec3(Color[0], Color[1], Color[2]);
    glUniform3fv(Renderer->ShaderProgram.Uniforms.LightColorUniformLoc, 1,
                 glm::value_ptr(LightColor));

    glm::vec3 LightPos = glm::vec3(Position[0], Position[1], Position[2]);
    glUniform3fv(Renderer->ShaderProgram.Uniforms.LightPositionUniformLoc, 1,
                 glm::value_ptr(LightPos));

    glUniform1fv(Renderer->ShaderProgram.Uniforms.AmbientStrengthUniformLoc, 1,
                 &AmbientStrength);
    glUniform1fv(Renderer->ShaderProgram.Uniforms.SpecularStrengthUniformLoc, 1,
                 &SpecularStrength);
}

void BeginMode3D(renderer *Renderer, camera *Camera, float ScreenWidth,
                 float ScreenHeight) {
    glm::mat4 View = CameraGetViewMatrix(Camera);
    glm::mat4 Projection = glm::perspective(
        glm::radians(Camera->Zoom), ScreenWidth / ScreenHeight, 0.1f, 100.0f);

    // Set view & projection for normal shader
    glUseProgram(Renderer->ShaderProgram.ID);
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(Renderer->ShaderProgram.Uniforms.ProjectionUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(Projection));

    glUniform3fv(Renderer->ShaderProgram.Uniforms.ViewPositionUniformLoc, 1,
                 glm::value_ptr(Camera->Position));

    // Set view & projection for outline shader
    glUseProgram(Renderer->OutlineShaderProgram.ID);
    // Sets the uniform value
    glUniformMatrix4fv(Renderer->OutlineShaderProgram.Uniforms.ViewUniformLoc,
                       1, GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(
        Renderer->OutlineShaderProgram.Uniforms.ProjectionUniformLoc, 1,
        GL_FALSE, glm::value_ptr(Projection));

    glUniform3fv(Renderer->OutlineShaderProgram.Uniforms.ViewPositionUniformLoc,
                 1, glm::value_ptr(Camera->Position));
}
