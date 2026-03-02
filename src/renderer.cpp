#include "renderer.h"

#include <string>
#include <fstream>
#include <iostream>
#include <cerrno>

#include "camera.h"
#include "mesh.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include "model.h"
#include "scene.h"
#include "texture.h"
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

renderer Renderer_Create(int ScreenWidth, int ScreenHeight) {
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);

    renderer Renderer;
    Renderer.ShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/default.vert", "./resources/shaders/default.frag");
    Renderer.OutlineShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/default.vert", "./resources/shaders/outline.frag");
    Renderer.QuadShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/default.vert", "./resources/shaders/quad.frag");
    Renderer.ScreenShaderProgram =
        Renderer_CreateShaderProgram("./resources/shaders/framebuffer.vert",
                                     "./resources/shaders/framebuffer.frag");
    Renderer.SkyBoxShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/cubemap.vert", "./resources/shaders/cubemap.frag");
    Renderer.InstanceShaderProgram =
        Renderer_CreateShaderProgram("./resources/shaders/instance.vert",
                                     "./resources/shaders/default.frag");

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
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // ### Screen Quad ###
    // Used for the framebuffer post-processing
    float ScreenQuadVertices[] = {
        // vertex attributes for a quad that fills the
        // entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
        0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
        1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f,
    };
    glGenVertexArrays(1, &Renderer.FrameBufferVAO);
    glGenBuffers(1, &Renderer.FrameBufferVBO);
    glBindVertexArray(Renderer.FrameBufferVAO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer.FrameBufferVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ScreenQuadVertices),
                 &ScreenQuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));

    glUseProgram(Renderer.ScreenShaderProgram.ID);
    glUniform1i(Renderer.ScreenShaderProgram.Uniforms.ScreenTextureUniformLoc,
                0);

    // ### Framebuffer Configuration ###
    glGenFramebuffers(1, &Renderer.FrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.FrameBuffer);
    // create a color attachment texture
    glGenTextures(1, &Renderer.TextureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, Renderer.TextureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ScreenWidth, ScreenHeight, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           Renderer.TextureColorBuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't
    // be sampling these)
    glGenRenderbuffers(1, &Renderer.RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, Renderer.RBO);

    // use a single renderbuffer object for
    // both a depth AND stencil buffer.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ScreenWidth,
                          ScreenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              Renderer.RBO); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we
    // want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                  << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return Renderer;
}

void Renderer_Destroy(renderer &Renderer) {
    glDeleteVertexArrays(1, &TriangleMesh.VAO);
    glDeleteBuffers(1, &TriangleMesh.VBO);

    glDeleteVertexArrays(1, &QuadMesh.VAO);
    glDeleteBuffers(1, &QuadMesh.VBO);
    glDeleteBuffers(1, &QuadMesh.EBO);

    glDeleteProgram(Renderer.ShaderProgram.ID);
    glDeleteProgram(Renderer.OutlineShaderProgram.ID);
    glDeleteProgram(Renderer.QuadShaderProgram.ID);
    glDeleteProgram(Renderer.ScreenShaderProgram.ID);
    glDeleteProgram(Renderer.InstanceShaderProgram.ID);

    glDeleteFramebuffers(1, &Renderer.FrameBuffer);
    glDeleteTextures(1, &Renderer.TextureColorBuffer);
    glDeleteRenderbuffers(1, &Renderer.RBO);
}

void Renderer_ResizeFramebuffer(const renderer &Renderer, int ScreenWidth,
                                int ScreenHeight) {
    if (ScreenWidth <= 0 || ScreenHeight <= 0) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, Renderer.TextureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ScreenWidth, ScreenHeight, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, Renderer.RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ScreenWidth,
                          ScreenHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.FrameBuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete after "
                     "resize!"
                  << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
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

    // Lights
    // DirectionalLight
    ShaderProgram.Uniforms.DirectionalLight.DirUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_dir_light.direction");
    ShaderProgram.Uniforms.DirectionalLight.AmbientUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_dir_light.ambient");
    ShaderProgram.Uniforms.DirectionalLight.DiffuseUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_dir_light.diffuse");
    ShaderProgram.Uniforms.DirectionalLight.SpecularUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_dir_light.specular");

    // PointLights
    uint32_t NumPointLights = 4;
    char buffer[100];
    for (int i = 0; i < NumPointLights; ++i) {
        point_light &PointLight = ShaderProgram.Uniforms.PointLights[i];

        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].position", i);
        PointLight.PositionUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].ambient", i);
        PointLight.AmbientUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].diffuse", i);
        PointLight.DiffuseUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].specular", i);
        PointLight.SpecularUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].constant", i);
        PointLight.ConstantUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].linear", i);
        PointLight.LinearUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].quadratic", i);
        PointLight.QuadraticUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
    }

    // SpotLight
    ShaderProgram.Uniforms.SpotLight.PositionUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.position");
    ShaderProgram.Uniforms.SpotLight.DirectionUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.direction");
    ShaderProgram.Uniforms.SpotLight.AmbientUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.ambient");
    ShaderProgram.Uniforms.SpotLight.DiffuseUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.diffuse");
    ShaderProgram.Uniforms.SpotLight.SpecularUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.specular");
    ShaderProgram.Uniforms.SpotLight.ConstantUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.constant");
    ShaderProgram.Uniforms.SpotLight.LinearUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.linear");
    ShaderProgram.Uniforms.SpotLight.QuadraticUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.quadratic");
    ShaderProgram.Uniforms.SpotLight.CutOffUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.cut_off");
    ShaderProgram.Uniforms.SpotLight.OuterCutOffUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.outer_cut_off");

    // Material
    ShaderProgram.Uniforms.Material.ShininessUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_material.shininess");
    ShaderProgram.Uniforms.Material.DiffuseUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_material.diffuse");
    ShaderProgram.Uniforms.Material.SpecularUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_material.specular");
    ShaderProgram.Uniforms.Material.NormalUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_material.normal");
    ShaderProgram.Uniforms.Material.HasNormalUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_material.has_normal");
    ShaderProgram.Uniforms.Material.HasSpecularUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_material.has_specular");

    // ScreenTexture
    ShaderProgram.Uniforms.ScreenTextureUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_screen_texture");

    return ShaderProgram;
}

void Renderer_ClearBackground(float R, float G, float B, float Alpha) {
    glClearColor(R, G, B, Alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer_DrawScene(const renderer &Renderer, const scene &Scene,
                        const context &Context) {

    // 1st. Pass: Draw the scene to the framebuffer
    // bind to framebuffer and draw scene as we normally would to color
    // texture
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.FrameBuffer);
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);

    // enable depth testing (is disabled for
    // rendering screen-space quad)
    glEnable(GL_DEPTH_TEST);
    // make sure we clear the framebuffer's content
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Sets the view & projection uniforms for all the programs
    Renderer_SetCameraUniforms(Renderer, Context.Camera, Context.ScreenWidth,
                               Context.ScreenHeight);

    // Entities
    for (entity Entity : Scene.Entities) {
        switch (Entity.Type) {
        case entity_type::Cube:
            Renderer_DrawCube(Renderer, Entity.Position, Entity.Rotation,
                              Entity.Color, Entity.Material, Entity.IsSelected);
            break;
        case entity_type::CubeMesh:
            Renderer_DrawCubeMesh(Renderer, Entity.Position, Entity.Rotation,
                                  Entity.Color, Entity.Mesh, Entity.IsSelected);
            break;
        case entity_type::Model:
            Renderer_DrawModel(Renderer, Entity.Position, Entity.Scale,
                               Entity.Rotation, Entity.Color, Entity.Model,
                               Entity.IsSelected);
            break;
        case entity_type::Triangle:
            Renderer_DrawTriangle(Renderer, Entity.Position);
            break;
        case entity_type::Quad:
            Renderer_DrawQuad(Renderer, Entity.Position, Entity.Material);
            break;
        case entity_type::QuadMesh:
            Renderer_DrawQuadMesh(Renderer, Entity.Position, Entity.Mesh);
            break;
        }
    }
    Renderer_DrawSceneLights(Renderer, Renderer.ShaderProgram, Scene,
                             Context.Camera);

    glUseProgram(Renderer.InstanceShaderProgram.ID);
    model Model = Scene.Instances[0].Model;
    Model_DrawInstances(Renderer.InstanceShaderProgram.ID, &Model,
                        Scene.Instances.size());

    Renderer_DrawSceneLights(Renderer, Renderer.InstanceShaderProgram, Scene,
                             Context.Camera);

    // Skybox
    Renderer_DrawSkybox(Renderer, Scene.Skybox);

    // 2nd. Pass: Draw whatever is in the Framebuffer to the screen quad
    // now bind back to default framebuffer and draw a quad plane with the
    // attached framebuffer color texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);
    // disable depth test so screen-space quad
    // isn't discarded due to depth test.
    glDisable(GL_DEPTH_TEST);
    // clear all relevant buffers
    // set clear color to white (not really necessary actually,
    // since we won't be able to see behind the quad anyways)
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(Renderer.ScreenShaderProgram.ID);
    glBindVertexArray(Renderer.FrameBufferVAO);

    // use the color attachment texture as
    // the texture of the quad plane
    glBindTexture(GL_TEXTURE_2D, Renderer.TextureColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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

void Renderer_DrawQuad(const renderer &Renderer, glm::vec<3, float> Position,
                       material Material) {
    glUseProgram(Renderer.ShaderProgram.ID);

    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.DiffuseUniformLoc, 0);
    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.SpecularUniformLoc, 1);
    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.NormalUniformLoc, 2);
    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.HasNormalUniformLoc,
                Material.HasNormalMap ? 1 : 0);
    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.HasSpecularUniformLoc,
                Material.HasSpecularMap ? 1 : 0);

    Texture_Bind(&Material.DiffuseMap, GL_TEXTURE0);
    Texture_Bind(&Material.SpecularMap, GL_TEXTURE1);
    Texture_Bind(&Material.NormalMap, GL_TEXTURE2);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position);

    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(Model));

    glBindVertexArray(QuadMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer_DrawQuadMesh(const renderer &Renderer,
                           glm::vec<3, float> Position, mesh Mesh) {
    glDisable(GL_CULL_FACE);
    glUseProgram(Renderer.QuadShaderProgram.ID);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position);

    glUniformMatrix4fv(Renderer.QuadShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(Model));

    Mesh_Draw(Renderer.QuadShaderProgram.ID, &Mesh);
    glEnable(GL_CULL_FACE);
}

void Renderer_DrawCube(const renderer &Renderer, glm::vec<3, float> Position,
                       glm::vec<4, float> Rotation, glm::vec<4, float> Color,
                       material Material, bool IsSelected) {
    // 1st render pass
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    glUseProgram(Renderer.ShaderProgram.ID);

    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.DiffuseUniformLoc, 0);
    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.SpecularUniformLoc, 1);
    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.NormalUniformLoc, 2);
    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.HasNormalUniformLoc,
                Material.HasNormalMap ? 1 : 0);
    glUniform1i(Renderer.ShaderProgram.Uniforms.Material.HasSpecularUniformLoc,
                Material.HasSpecularMap ? 1 : 0);

    Texture_Bind(&Material.DiffuseMap, GL_TEXTURE0);
    Texture_Bind(&Material.SpecularMap, GL_TEXTURE1);
    Texture_Bind(&Material.NormalMap, GL_TEXTURE2);

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
        glStencilMask(0xFF);
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
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }
}

void Renderer_DrawCubeMesh(const renderer &Renderer,
                           glm::vec<3, float> Position,
                           glm::vec<4, float> Rotation,
                           glm::vec<4, float> Color, mesh CubeMesh,
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

    Mesh_Draw(Renderer.ShaderProgram.ID, &CubeMesh);

    if (IsSelected) {
        // 2st render pass: draws the outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0xFF);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(Renderer.OutlineShaderProgram.ID);

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Position);
        Model = glm::scale(Model, glm::vec3(1.02f));
        Model = glm::rotate(Model, glm::radians(Rotation[0]), RotationVec);
        glUniformMatrix4fv(
            Renderer.OutlineShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
            glm::value_ptr(Model));

        Mesh_Draw(Renderer.OutlineShaderProgram.ID, &CubeMesh);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }
}

void Renderer_DrawModel(const renderer &Renderer, glm::vec<3, float> Position,
                        glm::vec<3, float> Scale, glm::vec<4, float> Rotation,
                        glm::vec<4, float> Color, model EntityModel,
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
    Model = glm::scale(Model, Scale);

    glm::vec3 RotationVec = glm::vec3(Rotation[1], Rotation[2], Rotation[3]);
    Model = glm::rotate(Model, glm::radians(Rotation[0]), RotationVec);
    glUniformMatrix4fv(Renderer.ShaderProgram.Uniforms.ModelUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(Model));

    Model_Draw(Renderer.ShaderProgram.ID, &EntityModel);

    if (IsSelected) {
        // 2st render pass: draws the outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0xFF);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(Renderer.OutlineShaderProgram.ID);

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Position);
        Model = glm::scale(Model, Scale + 0.01f);
        Model = glm::rotate(Model, glm::radians(Rotation[0]), RotationVec);
        glUniformMatrix4fv(
            Renderer.OutlineShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
            glm::value_ptr(Model));

        Model_Draw(Renderer.OutlineShaderProgram.ID, &EntityModel);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
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

void Renderer_DrawSkybox(const renderer &Renderer, const skybox &Skybox) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);
    glUseProgram(Renderer.SkyBoxShaderProgram.ID);

    mesh SkyboxMesh = Skybox.Mesh;
    Mesh_Draw(Renderer.SkyBoxShaderProgram.ID, &SkyboxMesh);

    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}

void Renderer_DrawSceneLights(const renderer &Renderer,
                              shader_program ShaderProgram, const scene &Scene,
                              const camera &Camera) {
    glUseProgram(ShaderProgram.ID);
    glUniform1f(ShaderProgram.Uniforms.Material.ShininessUniformLoc, 32.0f);

    // Lights
    for (size_t i = 0; i < Scene.Lights.size(); i++) {
        switch (Scene.Lights[i].LightType) {
        case light_type::Directional: {
            // Directional light
            float DirectionalLight[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            glm::vec3 DirectionalLightDir = glm::vec3(
                DirectionalLight[0], DirectionalLight[1], DirectionalLight[2]);
            glm::vec3 DirectionalLightAmbient = glm::vec3(0.05f, 0.05f, 0.05f);
            glm::vec3 DirectionalLightDiffuse = glm::vec3(0.4f, 0.4f, 0.4f);
            glm::vec3 DirectionalLightSpecular = glm::vec3(0.5f, 0.5f, 0.5f);

            glUniform3fv(ShaderProgram.Uniforms.DirectionalLight.DirUniformLoc,
                         1, glm::value_ptr(DirectionalLightDir));
            glUniform3fv(
                ShaderProgram.Uniforms.DirectionalLight.AmbientUniformLoc, 1,
                glm::value_ptr(DirectionalLightAmbient));
            glUniform3fv(
                ShaderProgram.Uniforms.DirectionalLight.DiffuseUniformLoc, 1,
                glm::value_ptr(DirectionalLightDiffuse));
            glUniform3fv(
                ShaderProgram.Uniforms.DirectionalLight.SpecularUniformLoc, 1,
                glm::value_ptr(DirectionalLightSpecular));
            break;
        }
        case light_type::Point: {
            glm::vec3 PointLightAmbient = glm::vec3(
                Scene.Lights[i].Entity.Color.r, Scene.Lights[i].Entity.Color.g,
                Scene.Lights[i].Entity.Color.b);
            glm::vec3 PointLightDiffuse = glm::vec3(0.8f, 0.8f, 0.8f);
            glm::vec3 PointLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
            glUniform3fv(
                ShaderProgram.Uniforms.PointLights[i].PositionUniformLoc, 1,
                glm::value_ptr(Scene.Lights[0].Entity.Position));
            glUniform3fv(
                ShaderProgram.Uniforms.PointLights[i].AmbientUniformLoc, 1,
                glm::value_ptr(PointLightAmbient));
            glUniform3fv(
                ShaderProgram.Uniforms.PointLights[i].DiffuseUniformLoc, 1,
                glm::value_ptr(PointLightDiffuse));
            glUniform3fv(
                ShaderProgram.Uniforms.PointLights[i].SpecularUniformLoc, 1,
                glm::value_ptr(PointLightSpecular));

            float Constant = 1.0f;
            float Linear = 0.09f;
            float Quadratic = 0.032f;
            glUniform1f(
                ShaderProgram.Uniforms.PointLights[i].ConstantUniformLoc,
                Constant);
            glUniform1f(ShaderProgram.Uniforms.PointLights[i].LinearUniformLoc,
                        Linear);
            glUniform1f(
                ShaderProgram.Uniforms.PointLights[i].QuadraticUniformLoc,
                Quadratic);
            break;
        }
        case light_type::Spot: {
            glm::vec3 SpotLightAmbient = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 SpotLightDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 SpotLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
            glUniform3fv(ShaderProgram.Uniforms.SpotLight.PositionUniformLoc, 1,
                         glm::value_ptr(Camera.Position));
            glUniform3fv(ShaderProgram.Uniforms.SpotLight.DirectionUniformLoc,
                         1, glm::value_ptr(Camera.Front));
            glUniform3fv(ShaderProgram.Uniforms.SpotLight.AmbientUniformLoc, 1,
                         glm::value_ptr(SpotLightAmbient));
            glUniform3fv(ShaderProgram.Uniforms.SpotLight.DiffuseUniformLoc, 1,
                         glm::value_ptr(SpotLightDiffuse));
            glUniform3fv(ShaderProgram.Uniforms.SpotLight.SpecularUniformLoc, 1,
                         glm::value_ptr(SpotLightSpecular));

            float Constant = 1.0f;
            float Linear = 0.09f;
            float Quadratic = 0.032f;
            float CutOff = glm::cos(glm::radians(12.5f));
            float OuterCutOff = glm::cos(glm::radians(15.0f));
            glUniform1f(ShaderProgram.Uniforms.SpotLight.ConstantUniformLoc,
                        Constant);
            glUniform1f(ShaderProgram.Uniforms.SpotLight.LinearUniformLoc,
                        Linear);
            glUniform1f(ShaderProgram.Uniforms.SpotLight.QuadraticUniformLoc,
                        Quadratic);
            glUniform1f(ShaderProgram.Uniforms.SpotLight.CutOffUniformLoc,
                        CutOff);
            glUniform1f(ShaderProgram.Uniforms.SpotLight.OuterCutOffUniformLoc,
                        OuterCutOff);
            break;
        }
        }
    }
}

void Renderer_SetCameraUniforms(const renderer &Renderer, const camera &Camera,
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

    // Set view & projection for quad shader
    glUseProgram(Renderer.QuadShaderProgram.ID);
    // Sets the uniform value
    glUniformMatrix4fv(Renderer.QuadShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(Renderer.QuadShaderProgram.Uniforms.ProjectionUniformLoc,
                       1, GL_FALSE, glm::value_ptr(Projection));

    glUniform3fv(Renderer.QuadShaderProgram.Uniforms.ViewPositionUniformLoc, 1,
                 glm::value_ptr(Camera.Position));

    // Set view & projection for instance shader
    glUseProgram(Renderer.InstanceShaderProgram.ID);
    glUniformMatrix4fv(Renderer.InstanceShaderProgram.Uniforms.ViewUniformLoc,
                       1, GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(
        Renderer.InstanceShaderProgram.Uniforms.ProjectionUniformLoc, 1,
        GL_FALSE, glm::value_ptr(Projection));

    glUniform3fv(Renderer.InstanceShaderProgram.Uniforms.ViewPositionUniformLoc,
                 1, glm::value_ptr(Camera.Position));

    // Set view & projection for skybox shader
    View = glm::mat4(glm::mat3(View));
    glUseProgram(Renderer.SkyBoxShaderProgram.ID);
    // Sets the uniform value
    glUniformMatrix4fv(Renderer.SkyBoxShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(
        Renderer.SkyBoxShaderProgram.Uniforms.ProjectionUniformLoc, 1, GL_FALSE,
        glm::value_ptr(Projection));

    glUniform3fv(Renderer.SkyBoxShaderProgram.Uniforms.ViewPositionUniformLoc,
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
            // positions         // normals           // texture coords
            -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,
            0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 0.0f,
            0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f,
            0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f,
            -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
            0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

            -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 1.0f,
            0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f, 1.0f,
            0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 0.0f,
            0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 1.0f,

            -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
            0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        },
    };
}
