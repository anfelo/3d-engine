#include "renderer.h"

#include <string>
#include <fstream>
#include <iostream>
#include <cerrno>

#include "camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

triangle_mesh TriangleMesh = Renderer_GetTriangleMesh();
quad_mesh QuadMesh = Renderer_GetQuadMesh();
cube_mesh CubeMesh = Renderer_GetCubeMesh();

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

renderer Renderer_Create() {
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    renderer Renderer;
    Renderer.ShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/default.vert", "./resources/shaders/default.frag");
    Renderer.OutlineShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/default.vert", "./resources/shaders/outline.frag");

    // ### Triangle ###
    glGenBuffers(1, &TriangleMesh.VBO);
    glGenVertexArrays(1, &TriangleMesh.VAO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(TriangleMesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, TriangleMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleMesh.Vertices),
                 TriangleMesh.Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ### Rectangle ###
    glGenBuffers(1, &QuadMesh.VBO);
    glGenVertexArrays(1, &QuadMesh.VAO);
    glGenBuffers(1, &QuadMesh.EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(QuadMesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, QuadMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadMesh.Vertices), QuadMesh.Vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QuadMesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QuadMesh.Indices),
                 QuadMesh.Indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ### Cube ###
    glGenBuffers(1, &CubeMesh.VBO);
    glGenVertexArrays(1, &CubeMesh.VAO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(CubeMesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, CubeMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeMesh.Vertices), CubeMesh.Vertices,
                 GL_STATIC_DRAW);
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

void Renderer_Destroy(renderer &Renderer) {
    glDeleteVertexArrays(1, &TriangleMesh.VAO);
    glDeleteBuffers(1, &TriangleMesh.VBO);

    glDeleteVertexArrays(1, &QuadMesh.VAO);
    glDeleteBuffers(1, &QuadMesh.VBO);
    glDeleteBuffers(1, &QuadMesh.EBO);

    glDeleteProgram(Renderer.ShaderProgram.ID);
}

shader_program Renderer_CreateShaderProgram(const char *VertexFile,
                                            const char *FragmentFile) {
    shader_program ShaderProgram;

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
    ShaderProgram.ID = glCreateProgram();
    glAttachShader(ShaderProgram.ID, VertexShader);
    glAttachShader(ShaderProgram.ID, FragmentShader);
    glLinkProgram(ShaderProgram.ID);
    // check for linking errors
    glGetProgramiv(ShaderProgram.ID, GL_LINK_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram.ID, 512, NULL, InfoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << InfoLog << std::endl;
    }
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    ShaderProgram.Uniforms.ModelUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_model");
    ShaderProgram.Uniforms.ViewUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_view");
    ShaderProgram.Uniforms.ProjectionUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_projection");

    // Fragment Shader Uniform Locators
    // INFO: This are only set on the normal shader
    ShaderProgram.Uniforms.EntityColorUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_entity_color");
    ShaderProgram.Uniforms.LightColorUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_light_color");
    ShaderProgram.Uniforms.LightPositionUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_light_pos");
    ShaderProgram.Uniforms.ViewPositionUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_view_pos");
    ShaderProgram.Uniforms.AmbientStrengthUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_ambient_strength");
    ShaderProgram.Uniforms.SpecularStrengthUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_specular_strength");

    return ShaderProgram;
}

void Renderer_ClearBackground(float R, float G, float B, float Alpha) {
    glClearColor(R, G, B, Alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer_DrawTriangle(const renderer &Renderer,
                           glm::vec<3, float> position) {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection;
    projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Sets the uniform value
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ProjectionUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(model));

    glUseProgram(Renderer.ShaderProgram.ID);
    glBindVertexArray(TriangleMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer_DrawQuad(const renderer &Renderer, glm::vec<3, float> position) {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection;
    projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Sets the uniform value
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ProjectionUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(model));

    glUseProgram(Renderer.ShaderProgram.ID);
    glBindVertexArray(QuadMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer_DrawCube(const renderer &Renderer, glm::vec<3, float> Position,
                       glm::vec<4, float> Rotation, glm::vec<4, float> Color,
                       bool IsSelected) {
    // 1st render pass
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    glUseProgram(Renderer.ShaderProgram.ID);

    glm::vec3 CubeColor = glm::vec3(Color[0], Color[1], Color[2]);
    glUniform3fv(Renderer.ShaderProgram.Uniforms.EntityColorUniformLoc, 1,
                 glm::value_ptr(CubeColor));

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position);
    Model = glm::scale(Model, glm::vec3(1.0f));

    glm::vec3 RotationVec = glm::vec3(Rotation[1], Rotation[2], Rotation[3]);
    Model = glm::rotate(Model, glm::radians(Rotation[0]), RotationVec);
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(Model));

    glBindVertexArray(CubeMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    if (IsSelected) {
        // 2st render pass: draws the outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(Renderer.OutlineShaderProgram.ID);

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Position);
        Model = glm::scale(Model, glm::vec3(1.02f));
        Model = glm::rotate(Model, glm::radians(Rotation[0]), RotationVec);
        glUniformMatrix4fv(
            Renderer.OutlineShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
            glm::value_ptr(Model));

        glBindVertexArray(CubeMesh.VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }
}

void Renderer_DrawLight(const renderer &Renderer, glm::vec<3, float> Position,
                        glm::vec<4, float> Color, float AmbientStrength,
                        float SpecularStrength) {
    glUseProgram(Renderer.ShaderProgram.ID);

    glm::vec3 LightColor = glm::vec3(Color[0], Color[1], Color[2]);
    glUniform3fv(Renderer.ShaderProgram.Uniforms.LightColorUniformLoc, 1,
                 glm::value_ptr(LightColor));

    glm::vec3 LightPos = glm::vec3(Position[0], Position[1], Position[2]);
    glUniform3fv(Renderer.ShaderProgram.Uniforms.LightPositionUniformLoc, 1,
                 glm::value_ptr(LightPos));

    glUniform1fv(Renderer.ShaderProgram.Uniforms.AmbientStrengthUniformLoc, 1,
                 &AmbientStrength);
    glUniform1fv(Renderer.ShaderProgram.Uniforms.SpecularStrengthUniformLoc, 1,
                 &SpecularStrength);
}

void Renderer_DrawScene(const renderer &Renderer, const scene &Scene,
                        const camera &Camera) {
    for (entity Entity : Scene.Entities) {
        switch (Entity.Type) {
        case entity_type::Cube:
            Renderer_DrawCube(Renderer, Entity.Position, Entity.Rotation,
                              Entity.Color, Entity.IsSelected);
            break;
        case entity_type::Triangle:
            Renderer_DrawTriangle(Renderer, Entity.Position);
            break;
        case entity_type::Quad:
            Renderer_DrawQuad(Renderer, Entity.Position);
            break;
        }
    }

    for (light_entity Light : Scene.Lights) {
        Renderer_DrawLight(Renderer, Light.Position, Light.Color,
                           Light.AmbientStrength, Light.SpecularStrength);
    }
}

void Renderer_BeginMode3D(const renderer &Renderer, const camera &Camera,
                          float ScreenWidth, float ScreenHeight) {
    glm::mat4 View = Camera_GetViewMatrix(Camera);
    glm::mat4 Projection = glm::perspective(
        glm::radians(Camera.Zoom), ScreenWidth / ScreenHeight, 0.1f, 100.0f);

    // Set view & projection for normal shader
    glUseProgram(Renderer.ShaderProgram.ID);
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ProjectionUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(Projection));

    glUniform3fv(Renderer.ShaderProgram.Uniforms.ViewPositionUniformLoc, 1,
                 glm::value_ptr(Camera.Position));

    // Set view & projection for outline shader
    glUseProgram(Renderer.OutlineShaderProgram.ID);
    // Sets the uniform value
    glUniformMatrix4fv(Renderer.OutlineShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(
        Renderer.OutlineShaderProgram.Uniforms.ProjectionUniformLoc, 1,
        GL_FALSE, glm::value_ptr(Projection));

    glUniform3fv(Renderer.OutlineShaderProgram.Uniforms.ViewPositionUniformLoc,
                 1, glm::value_ptr(Camera.Position));
}

triangle_mesh Renderer_GetTriangleMesh() {
    return triangle_mesh{
        .Vertices{-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f},
    };
}

quad_mesh Renderer_GetQuadMesh() {
    return quad_mesh{
        .Vertices{
            0.5f, 0.5f, 0.0f,   // top right
            0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f, // bottom left
            -0.5f, 0.5f, 0.0f   // top left
        },
        .Indices{0, 1, 3, 1, 2, 3},
    };
}

cube_mesh Renderer_GetCubeMesh() {
    return cube_mesh{
        .Vertices{
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
            0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
        },
    };
}
