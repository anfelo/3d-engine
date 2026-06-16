#include "renderer.h"

#include <string>
#include <fstream>
#include <iostream>
#include <cerrno>

#include "camera.h"
#include "entity.h"
#include "material.h"
#include "mesh.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include "model.h"
#include "scene.h"
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

renderer Renderer_Create(const context &Context) {
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
    Renderer.UnlitShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/unlit.vert", "./resources/shaders/unlit.frag");
    Renderer.SimpleDepthShaderProgram =
        Renderer_CreateShaderProgram("./resources/shaders/simple_depth.vert",
                                     "./resources/shaders/simple_depth.frag");
    Renderer.CubemapDepthShaderProgram =
        Renderer_CreateShaderProgram("./resources/shaders/cube_depth.vert",
                                     "./resources/shaders/cube_depth.frag",
                                     "./resources/shaders/cube_depth.gs");
    Renderer.BlurShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/blur.vert", "./resources/shaders/blur.frag");
    Renderer.WaterShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/water.vert", "./resources/shaders/water.frag");
    Renderer.GuiShaderProgram = Renderer_CreateShaderProgram(
        "./resources/shaders/gui.vert", "./resources/shaders/gui.frag");

    glUseProgram(Renderer.BlurShaderProgram.ID);
    glUniform1i(Renderer.BlurShaderProgram.Uniforms.ScreenTextureUniformLoc, 0);

    glUseProgram(Renderer.WaterShaderProgram.ID);
    glUniform1i(
        Renderer.WaterShaderProgram.Uniforms.RefractionTextureUniformLoc, 2);
    glUniform1i(
        Renderer.WaterShaderProgram.Uniforms.ReflectionTextureUniformLoc, 3);
    glUniform1i(Renderer.WaterShaderProgram.Uniforms.DepthMapUniformLoc, 4);

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
    glUniform1i(Renderer.ScreenShaderProgram.Uniforms.BloomTextureUniformLoc,
                1);

    // ### Framebuffer Configuration ###
    glGenFramebuffers(1, &Renderer.FrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.FrameBuffer);
    // create a color attachment texture
    glGenTextures(1, &Renderer.TextureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, Renderer.TextureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Context.ScreenWidth,
                 Context.ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // we clamp to the edge as the blur filter would otherwise sample repeated
    // texture values!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           Renderer.TextureColorBuffer, 0);

    // create another color attachment texture for only the bright colors
    glGenTextures(1, &Renderer.BrightColorBuffer);
    glBindTexture(GL_TEXTURE_2D, Renderer.BrightColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Context.ScreenWidth,
                 Context.ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // we clamp to the edge as the blur filter would otherwise sample repeated
    // texture values!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                           Renderer.BrightColorBuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't
    // be sampling these)
    glGenRenderbuffers(1, &Renderer.RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, Renderer.RBO);

    // use a single renderbuffer object for
    // both a depth AND stencil buffer.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                          Context.ScreenWidth, Context.ScreenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, Renderer.RBO);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for
    // rendering
    unsigned int Attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, Attachments);
    // now that we actually created the framebuffer and added all attachments we
    // want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                  << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    glGenFramebuffers(2, Renderer.PingPongFBO);
    glGenTextures(2, Renderer.PingPongColorBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, Renderer.PingPongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, Renderer.PingPongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Context.ScreenWidth,
                     Context.ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // we clamp to the edge as the blur filter would
        // otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, Renderer.PingPongColorBuffers[i],
                               0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
    }

    // ### Depth Map Configuration ###
    glGenFramebuffers(1, &Renderer.DepthMapFBO);

    glGenTextures(1, &Renderer.DepthMapBuffer);
    glBindTexture(GL_TEXTURE_2D, Renderer.DepthMapBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 Context.ShadowbufferWidth, Context.ShadowbufferHeight, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float BorderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.DepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           Renderer.DepthMapBuffer, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ### Cube Depth Map Configuration ###
    glGenFramebuffers(1, &Renderer.DepthCubemapFBO);

    glGenTextures(1, &Renderer.DepthCubemapBuffer);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Renderer.DepthCubemapBuffer);

    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     Context.ShadowbufferWidth, Context.ShadowbufferHeight, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.DepthCubemapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         Renderer.DepthCubemapBuffer, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ### Water Buffers Configuration ###
    // Refraction
    Renderer.RefractionFBOWidth = 1280;
    Renderer.RefractionFBOHeight = 720;
    glGenFramebuffers(1, &Renderer.RefractionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.RefractionFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glGenTextures(1, &Renderer.RefractionColorBuffer);
    glBindTexture(GL_TEXTURE_2D, Renderer.RefractionColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Renderer.RefractionFBOWidth,
                 Renderer.RefractionFBOHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           Renderer.RefractionColorBuffer, 0);

    glGenTextures(1, &Renderer.RefractionDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, Renderer.RefractionDepthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 Renderer.RefractionFBOWidth, Renderer.RefractionFBOHeight, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           Renderer.RefractionDepthBuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout
            << "ERROR::FRAMEBUFFER:: Refraction framebuffer is not complete!"
            << std::endl;
    }

    // Reflection
    Renderer.ReflectionFBOWidth = 320;
    Renderer.ReflectionFBOHeight = 180;
    glGenFramebuffers(1, &Renderer.ReflectionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.ReflectionFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glGenTextures(1, &Renderer.ReflectionColorBuffer);
    glBindTexture(GL_TEXTURE_2D, Renderer.ReflectionColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Renderer.ReflectionFBOWidth,
                 Renderer.ReflectionFBOHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           Renderer.ReflectionColorBuffer, 0);

    glGenRenderbuffers(1, &Renderer.ReflectionRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, Renderer.ReflectionRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          Renderer.ReflectionFBOWidth,
                          Renderer.ReflectionFBOHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, Renderer.ReflectionRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout
            << "ERROR::FRAMEBUFFER:: Reflection framebuffer is not complete!"
            << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return Renderer;
}

void Renderer_Destroy(renderer &Renderer) {
    glDeleteProgram(Renderer.ShaderProgram.ID);
    glDeleteProgram(Renderer.OutlineShaderProgram.ID);
    glDeleteProgram(Renderer.QuadShaderProgram.ID);
    glDeleteProgram(Renderer.ScreenShaderProgram.ID);
    glDeleteProgram(Renderer.InstanceShaderProgram.ID);

    glDeleteFramebuffers(1, &Renderer.FrameBuffer);
    glDeleteTextures(1, &Renderer.TextureColorBuffer);
    glDeleteRenderbuffers(1, &Renderer.RBO);

    glDeleteFramebuffers(1, &Renderer.RefractionFBO);
    glDeleteTextures(1, &Renderer.RefractionColorBuffer);
    glDeleteTextures(1, &Renderer.RefractionDepthBuffer);

    glDeleteFramebuffers(1, &Renderer.ReflectionFBO);
    glDeleteTextures(1, &Renderer.ReflectionColorBuffer);
    glDeleteRenderbuffers(1, &Renderer.ReflectionRBO);
}

void Renderer_ResizeFramebuffer(const renderer &Renderer, int ScreenWidth,
                                int ScreenHeight) {
    if (ScreenWidth <= 0 || ScreenHeight <= 0) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, Renderer.TextureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ScreenWidth, ScreenHeight, 0,
                 GL_RGBA, GL_FLOAT, NULL);

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
                                            const char *FragmentFile,
                                            const char *GeometryFile) {
    shader_program ShaderProgram;

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
    ShaderProgram.ID = glCreateProgram();
    glAttachShader(ShaderProgram.ID, VertexShader);
    glAttachShader(ShaderProgram.ID, FragmentShader);
    if (GeometryFile) {
        glAttachShader(ShaderProgram.ID, GeometryShader);
    }

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
    if (GeometryFile) {
        glDeleteShader(GeometryShader);
    }

    ShaderProgram.Uniforms.ModelUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_model");
    ShaderProgram.Uniforms.ViewUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_view");
    ShaderProgram.Uniforms.ProjectionUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_projection");

    ShaderProgram.Uniforms.TimeUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_time");
    ShaderProgram.Uniforms.ClipPlaneUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_clip_plane");

    // LightSpace (for shadow mapping)
    ShaderProgram.Uniforms.LightSpaceMatrixUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_light_space_matrix");
    ShaderProgram.Uniforms.ShadowMapUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_shadow_map");
    ShaderProgram.Uniforms.ShadowCubemapUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_shadow_cubemap");

    ShaderProgram.Uniforms.FarPlaneUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_far_plane");
    ShaderProgram.Uniforms.NearPlaneUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_near_plane");
    ShaderProgram.Uniforms.ShadowMatricesUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_shadow_matrices");

    // HDR Uniform Locators
    ShaderProgram.Uniforms.HDRExposureUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_exposure");
    ShaderProgram.Uniforms.HDREnabledUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_hdr_enabled");

    // Bloom Filter Uniform Locators
    ShaderProgram.Uniforms.BloomTextureUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_bloom_texture");
    ShaderProgram.Uniforms.BloomEnabledUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_bloom_enabled");
    ShaderProgram.Uniforms.HorizontalUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_horizontal");

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
    ShaderProgram.Uniforms.DirectionalLight.EnabledUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_dir_light.enabled");
    ShaderProgram.Uniforms.DirectionalLight.UseBlinnUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_dir_light.blinn");
    ShaderProgram.Uniforms.DirectionalLight.CastsShadowUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_dir_light.casts_shadow");

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
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].enabled", i);
        PointLight.EnabledUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].blinn", i);
        PointLight.UseBlinnUniformLoc =
            glGetUniformLocation(ShaderProgram.ID, buffer);
        snprintf(buffer, sizeof(buffer), "u_point_lights[%d].casts_shadow", i);
        PointLight.CastsShadowUniformLoc =
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
    ShaderProgram.Uniforms.SpotLight.EnabledUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.enabled");
    ShaderProgram.Uniforms.SpotLight.UseBlinnUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.blinn");
    ShaderProgram.Uniforms.SpotLight.CastsShadowUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_spot_light.casts_shadow");

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
    ShaderProgram.Uniforms.EffectUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_effect");

    ShaderProgram.Uniforms.RefractionTextureUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_refraction_texture");
    ShaderProgram.Uniforms.ReflectionTextureUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_reflection_texture");
    ShaderProgram.Uniforms.DepthMapUniformLoc =
        glGetUniformLocation(ShaderProgram.ID, "u_depth_map");

    return ShaderProgram;
}

void Renderer_ClearBackground(float R, float G, float B, float Alpha) {
    glClearColor(R, G, B, Alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer_BindFramebuffer(const renderer &Renderer, GLuint FramebufferID,
                              int Width, int Height) {
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferID);
    glViewport(0, 0, Width, Height);
}

void Renderer_DrawSceneWater(const renderer &Renderer, const scene &Scene) {
    for (entity Entity : Scene.Entities) {
        if (Entity.Mesh.Material.ShaderMaterial != shader_material::Water) {
            continue;
        }

        Renderer_DrawQuadEntity(Renderer, Renderer.WaterShaderProgram, Entity);
    }
}

void Renderer_DrawScene(const renderer &Renderer,
                        const shader_program &ShaderProgram, const scene &Scene,
                        bool useEntityShader) {

    // Entities
    for (entity Entity : Scene.Entities) {
        shader_program Shader = ShaderProgram;
        if (useEntityShader) {
            switch (Entity.Mesh.Material.ShaderMaterial) {
            case shader_material::Default:
                Shader = Renderer.ShaderProgram;
                break;
            case shader_material::Unlit:
                Shader = Renderer.UnlitShaderProgram;
                break;
            case shader_material::Water:
                // INFO: Water is drawn through a different method
                continue;
                break;
            }
        }

        switch (Entity.Type) {
        case entity_type::Cube:
            // TODO: Merge this with the CubeMesh
            break;
        case entity_type::CubeMesh:
            Renderer_DrawCubeEntity(Renderer, Shader, Entity);
            break;
        case entity_type::Model:
            Renderer_DrawModelEntity(Renderer, Shader, Entity);
            break;
        case entity_type::Triangle:
            // TODO: Make sure this works
            Renderer_DrawTriangle(Renderer, Shader, Entity.Position);
            break;
        case entity_type::Quad:
            // TODO: Merge this with the QuadMesh
            break;
        case entity_type::QuadMesh:
            Renderer_DrawQuadEntity(Renderer, Shader, Entity);
            break;
        }
    }

    if (useEntityShader) {
        // Lights (Debug)
        for (light Light : Scene.Lights) {
            if (Light.ShowDebug && Light.LightType == light_type::Point) {
                shader_program Shader = ShaderProgram;
                switch (Light.Entity.Mesh.Material.ShaderMaterial) {
                case shader_material::Default:
                    Shader = Renderer.ShaderProgram;
                    break;
                case shader_material::Unlit:
                    Shader = Renderer.UnlitShaderProgram;
                    break;
                default:
                    break;
                }

                Renderer_DrawCubeEntity(Renderer, Shader, Light.Entity);
            }
        }
    }
}

void Renderer_Draw(const renderer &Renderer, const scene &Scene,
                   const context &Context) {
    glEnable(GL_DEPTH_TEST);

    // Directional Shadow Mapping
    // 1st. Pass: Draw the scene to the depth buffer for the shadow pass
    glViewport(0, 0, Context.ShadowbufferWidth, Context.ShadowbufferHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.DepthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set view & projection for depth shader from the light's perspective
    float NearPlane = 0.1f, FarPlane = 25.0f;
    glm::mat4 LightProjection =
        glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, NearPlane, FarPlane);

    glm::vec3 LightDir = glm::normalize(glm::vec3(1.0f, -1.0f, 0.3f));
    glm::vec3 LightPos = -LightDir * 10.0f;
    glm::mat4 LightView = glm::lookAt(LightPos, glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 LightSpaceMatrix = LightProjection * LightView;

    glUseProgram(Renderer.SimpleDepthShaderProgram.ID);
    glUniformMatrix4fv(
        Renderer.SimpleDepthShaderProgram.Uniforms.LightSpaceMatrixUniformLoc,
        1, GL_FALSE, glm::value_ptr(LightSpaceMatrix));

    // Remove peter panning problems
    // glCullFace(GL_FRONT);
    Renderer_DrawScene(Renderer, Renderer.SimpleDepthShaderProgram, Scene,
                       false);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Point Shadow Mapping
    glViewport(0, 0, Context.ShadowbufferWidth, Context.ShadowbufferHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.DepthCubemapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(Renderer.CubemapDepthShaderProgram.ID);

    // Set view & projection for depth shader from the light's perspective
    float Aspect =
        (float)Context.ShadowbufferWidth / (float)Context.ShadowbufferHeight;
    NearPlane = 0.1f;
    FarPlane = 25.0f;

    glm::mat4 PointLightProjection =
        glm::perspective(glm::radians(90.0f), Aspect, NearPlane, FarPlane);
    glm::vec3 PointLightPosition = Scene.Lights[0].Entity.Position;

    std::vector<glm::mat4> PointShadowTransforms;
    PointShadowTransforms.push_back(
        PointLightProjection *
        glm::lookAt(PointLightPosition,
                    PointLightPosition + glm::vec3(1.0, 0.0, 0.0),
                    glm::vec3(0.0, -1.0, 0.0)));
    PointShadowTransforms.push_back(
        PointLightProjection *
        glm::lookAt(PointLightPosition,
                    PointLightPosition + glm::vec3(-1.0, 0.0, 0.0),
                    glm::vec3(0.0, -1.0, 0.0)));
    PointShadowTransforms.push_back(
        PointLightProjection *
        glm::lookAt(PointLightPosition,
                    PointLightPosition + glm::vec3(0.0, 1.0, 0.0),
                    glm::vec3(0.0, 0.0, 1.0)));
    PointShadowTransforms.push_back(
        PointLightProjection *
        glm::lookAt(PointLightPosition,
                    PointLightPosition + glm::vec3(0.0, -1.0, 0.0),
                    glm::vec3(0.0, 0.0, -1.0)));
    PointShadowTransforms.push_back(
        PointLightProjection *
        glm::lookAt(PointLightPosition,
                    PointLightPosition + glm::vec3(0.0, 0.0, 1.0),
                    glm::vec3(0.0, -1.0, 0.0)));
    PointShadowTransforms.push_back(
        PointLightProjection *
        glm::lookAt(PointLightPosition,
                    PointLightPosition + glm::vec3(0.0, 0.0, -1.0),
                    glm::vec3(0.0, -1.0, 0.0)));

    glUniform3fv(
        Renderer.CubemapDepthShaderProgram.Uniforms.LightPositionUniformLoc, 1,
        glm::value_ptr(PointLightPosition));
    glUniform1fv(Renderer.CubemapDepthShaderProgram.Uniforms.FarPlaneUniformLoc,
                 1, &FarPlane);
    for (unsigned int i = 0; i < 6; ++i) {
        glUniformMatrix4fv(
            glGetUniformLocation(
                Renderer.CubemapDepthShaderProgram.ID,
                ("u_shadow_matrices[" + std::to_string(i) + "]").c_str()),
            1, GL_FALSE, glm::value_ptr(PointShadowTransforms[i]));
    }

    Renderer_DrawScene(Renderer, Renderer.CubemapDepthShaderProgram, Scene,
                       false);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);
    Renderer_BindFramebuffer(Renderer, Renderer.RefractionFBO,
                             Renderer.RefractionFBOWidth,
                             Renderer.RefractionFBOHeight);
    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec4 RefractionClipPlane = glm::vec4(0.0f, -1.0f, 0.0f, 0.01f);
    glUseProgram(Renderer.ShaderProgram.ID);
    glUniform4fv(Renderer.ShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(RefractionClipPlane));
    glUseProgram(Renderer.UnlitShaderProgram.ID);
    glUniform4fv(Renderer.UnlitShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(RefractionClipPlane));
    glUseProgram(Renderer.InstanceShaderProgram.ID);
    glUniform4fv(Renderer.InstanceShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(RefractionClipPlane));
    glUseProgram(Renderer.WaterShaderProgram.ID);
    glUniform4fv(Renderer.WaterShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(RefractionClipPlane));
    Renderer_SetCameraUniforms(Renderer, Context.Camera, Context.ScreenWidth,
                               Context.ScreenHeight);
    Renderer_SetSceneLightsUniforms(Renderer, Renderer.ShaderProgram, Scene,
                                    Context.Camera);
    Renderer_SetOtherUniforms(Renderer, Context);

    Renderer_DrawScene(Renderer, Renderer.ShaderProgram, Scene);
    Renderer_DrawSkybox(Renderer, Scene.Skybox);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);

    Renderer_BindFramebuffer(Renderer, Renderer.ReflectionFBO,
                             Renderer.ReflectionFBOWidth,
                             Renderer.ReflectionFBOHeight);
    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec4 ReflectionClipPlane = glm::vec4(0.0f, 1.0f, 0.0f, -0.01f);
    glUseProgram(Renderer.ShaderProgram.ID);
    glUniform4fv(Renderer.ShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(ReflectionClipPlane));
    glUseProgram(Renderer.UnlitShaderProgram.ID);
    glUniform4fv(Renderer.UnlitShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(ReflectionClipPlane));
    glUseProgram(Renderer.InstanceShaderProgram.ID);
    glUniform4fv(Renderer.InstanceShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(ReflectionClipPlane));
    glUseProgram(Renderer.WaterShaderProgram.ID);
    glUniform4fv(Renderer.WaterShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(ReflectionClipPlane));

    // Invert Camera position and pitch
    float Distance = 2 * (Context.Camera.Position.y); // - WaterHeight
    glm::vec3 NewPosition = glm::vec3(Context.Camera.Position.x,
                                      Context.Camera.Position.y - Distance,
                                      Context.Camera.Position.z);
    camera OtherCamera =
        Camera_Create(NewPosition, Context.Camera.Up, Context.Camera.Yaw,
                      -Context.Camera.Pitch);
    Renderer_SetCameraUniforms(Renderer, OtherCamera, Context.ScreenWidth,
                               Context.ScreenHeight);
    Renderer_SetSceneLightsUniforms(Renderer, Renderer.ShaderProgram, Scene,
                                    Context.Camera);
    Renderer_SetOtherUniforms(Renderer, Context);

    Renderer_DrawScene(Renderer, Renderer.ShaderProgram, Scene);
    Renderer_DrawSkybox(Renderer, Scene.Skybox);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);

    // 2st. Pass: Draw the scene to the framebuffer
    // bind to framebuffer and draw scene as we normally would to color
    // texture
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.FrameBuffer);
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);

    // enable depth testing (is disabled for
    // rendering screen-space quad)
    glEnable(GL_DEPTH_TEST);
    // make sure we clear the framebuffer's content
    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glDisable(GL_CLIP_DISTANCE0);
    // INFO: Hack when disabling the clip_distance doesn't work
    glm::vec4 NoClipPlane = glm::vec4(0.0f, 1.0f, 0.0f, 10000.0f);
    glUseProgram(Renderer.ShaderProgram.ID);
    glUniform4fv(Renderer.ShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(NoClipPlane));
    glUseProgram(Renderer.UnlitShaderProgram.ID);
    glUniform4fv(Renderer.UnlitShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(NoClipPlane));
    glUseProgram(Renderer.InstanceShaderProgram.ID);
    glUniform4fv(Renderer.InstanceShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(NoClipPlane));
    glUseProgram(Renderer.WaterShaderProgram.ID);
    glUniform4fv(Renderer.WaterShaderProgram.Uniforms.ClipPlaneUniformLoc, 1,
                 glm::value_ptr(NoClipPlane));

    glUseProgram(Renderer.ShaderProgram.ID);

    // Sets the view & projection uniforms for all the programs
    Renderer_SetCameraUniforms(Renderer, Context.Camera, Context.ScreenWidth,
                               Context.ScreenHeight);

    Renderer_SetSceneLightsUniforms(Renderer, Renderer.ShaderProgram, Scene,
                                    Context.Camera);

    Renderer_SetOtherUniforms(Renderer, Context);

    // Directional Shadow Map
    glActiveTexture(GL_TEXTURE3);
    glUniform1i(Renderer.ShaderProgram.Uniforms.ShadowMapUniformLoc, 3);
    glBindTexture(GL_TEXTURE_2D, Renderer.DepthMapBuffer);

    // Point Shadow Cubemap
    glActiveTexture(GL_TEXTURE4);
    glUniform1i(Renderer.ShaderProgram.Uniforms.ShadowCubemapUniformLoc, 4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Renderer.DepthCubemapBuffer);

    glUniformMatrix4fv(
        Renderer.ShaderProgram.Uniforms.LightSpaceMatrixUniformLoc, 1, GL_FALSE,
        glm::value_ptr(LightSpaceMatrix));
    glUniform1fv(Renderer.ShaderProgram.Uniforms.FarPlaneUniformLoc, 1,
                 &FarPlane);

    Renderer_DrawScene(Renderer, Renderer.ShaderProgram, Scene);

    // TODO: Refactor the Water Renderer
    glUseProgram(Renderer.WaterShaderProgram.ID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, Renderer.RefractionColorBuffer);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Renderer.ReflectionColorBuffer);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Renderer.RefractionDepthBuffer);

    NearPlane = 0.1f;
    FarPlane = 1000.0f;
    glUniform1f(Renderer.WaterShaderProgram.Uniforms.NearPlaneUniformLoc,
                NearPlane);
    glUniform1f(Renderer.WaterShaderProgram.Uniforms.FarPlaneUniformLoc,
                FarPlane);
    Renderer_SetSceneLightsUniforms(Renderer, Renderer.WaterShaderProgram,
                                    Scene, Context.Camera);
    glUseProgram(Renderer.ShaderProgram.ID);
    Renderer_DrawSceneWater(Renderer, Scene);

    // TODO: Keeping the instances out of the shadow pass for now.
    if (Scene.Instances.size() > 0) {
        glUseProgram(Renderer.InstanceShaderProgram.ID);

        Renderer_SetSceneLightsUniforms(
            Renderer, Renderer.InstanceShaderProgram, Scene, Context.Camera);

        // Directional Shadow Map
        glActiveTexture(GL_TEXTURE3);
        glUniform1i(Renderer.InstanceShaderProgram.Uniforms.ShadowMapUniformLoc,
                    3);
        glBindTexture(GL_TEXTURE_2D, Renderer.DepthMapBuffer);

        // Point Shadow Cubemap
        glActiveTexture(GL_TEXTURE4);
        glUniform1i(
            Renderer.InstanceShaderProgram.Uniforms.ShadowCubemapUniformLoc, 4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Renderer.DepthCubemapBuffer);

        glUniformMatrix4fv(
            Renderer.InstanceShaderProgram.Uniforms.LightSpaceMatrixUniformLoc,
            1, GL_FALSE, glm::value_ptr(LightSpaceMatrix));
        glUniform1fv(Renderer.InstanceShaderProgram.Uniforms.FarPlaneUniformLoc,
                     1, &FarPlane);

        model *Model = Scene.Instances[0].Model;
        Model_DrawInstances(Renderer.InstanceShaderProgram.ID, *Model,
                            Scene.Instances.size());
    }

    // Skybox
    Renderer_DrawSkybox(Renderer, Scene.Skybox);

    // Bloom Blur Pass: blur bright fragments with two-pass Gaussian Blur
    // --------------------------------------------------
    bool Horizontal = true, FirstIteration = true;
    unsigned int Amount = 10;
    glUseProgram(Renderer.BlurShaderProgram.ID);
    glBindVertexArray(Renderer.FrameBufferVAO);
    glActiveTexture(GL_TEXTURE0);
    for (unsigned int i = 0; i < Amount; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, Renderer.PingPongFBO[Horizontal]);
        glUniform1i(Renderer.BlurShaderProgram.Uniforms.HorizontalUniformLoc,
                    Horizontal);
        // bind texture of other framebuffer (or scene if first iteration)
        glBindTexture(GL_TEXTURE_2D,
                      FirstIteration
                          ? Renderer.BrightColorBuffer
                          : Renderer.PingPongColorBuffers[!Horizontal]);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        Horizontal = !Horizontal;
        if (FirstIteration) {
            FirstIteration = false;
        }
    }

    Renderer_BindFramebuffer(Renderer, Renderer.FrameBuffer,
                             Context.FramebufferWidth,
                             Context.FramebufferHeight);
    // Draw all GuiTextures into the scene framebuffer before it is
    // presented.
    glUseProgram(Renderer.GuiShaderProgram.ID);
    glm::vec2 ScreenSize =
        glm::vec2(Context.FramebufferWidth, Context.FramebufferHeight);
    glUniform2fv(
        glGetUniformLocation(Renderer.GuiShaderProgram.ID, "u_screen_size"), 1,
        glm::value_ptr(ScreenSize));
    for (unsigned int i = 0; i < Scene.GuiTextures.size(); ++i) {
        Renderer_DrawGuiEntity(Renderer, Renderer.GuiShaderProgram,
                               Scene.GuiTextures.at(i));
    }

    // 3th. Pass: Draw whatever is in the Framebuffer to the screen quad
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

    glUniform1iv(Renderer.ScreenShaderProgram.Uniforms.EffectUniformLoc, 1,
                 &Scene.Effect);
    glUniform1i(Renderer.ScreenShaderProgram.Uniforms.HDREnabledUniformLoc,
                Scene.HDREnabled ? 1 : 0);
    glUniform1fv(Renderer.ScreenShaderProgram.Uniforms.HDRExposureUniformLoc, 1,
                 &Scene.HDRExposure);
    glUniform1i(Renderer.ScreenShaderProgram.Uniforms.BloomEnabledUniformLoc,
                Scene.BloomEnabled ? 1 : 0);

    // use the color attachment texture as
    // the texture of the quad plane
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Renderer.TextureColorBuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Renderer.PingPongColorBuffers[!Horizontal]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// TODO: Needs fixing
void Renderer_DrawTriangle(const renderer &Renderer,
                           const shader_program &ShaderProgram,
                           glm::vec<3, float> position) {
    glUseProgram(ShaderProgram.ID);

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection;
    projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Sets the uniform value
    glUniformMatrix4fv(ShaderProgram.Uniforms.ViewUniformLoc, 1, GL_FALSE,
                       glm::value_ptr(view));
    glUniformMatrix4fv(ShaderProgram.Uniforms.ProjectionUniformLoc, 1, GL_FALSE,
                       glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    glUniformMatrix4fv(ShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
                       glm::value_ptr(model));

    // glBindVertexArray(TriangleMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer_DrawQuadEntity(const renderer &Renderer,
                             const shader_program &ShaderProgram,
                             const entity &Entity) {
    glDisable(GL_CULL_FACE);
    glUseProgram(ShaderProgram.ID);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Entity.Position);
    Model = glm::scale(Model, Entity.Scale);

    glm::vec3 RotationVec =
        glm::vec3(Entity.Rotation[1], Entity.Rotation[2], Entity.Rotation[3]);
    Model = glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);

    glm::vec4 Color = Entity.Mesh.Material.Color;
    glm::vec3 QuadColor = glm::vec3(Color[0], Color[1], Color[2]);
    glUniform3fv(ShaderProgram.Uniforms.EntityColorUniformLoc, 1,
                 glm::value_ptr(QuadColor));

    glUniformMatrix4fv(ShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
                       glm::value_ptr(Model));

    Mesh_Draw(ShaderProgram.ID, Entity.Mesh);
    glEnable(GL_CULL_FACE);
}

void Renderer_DrawGuiEntity(const renderer &Renderer,
                            const shader_program &ShaderProgram,
                            const entity &Entity) {
    glDisable(GL_CULL_FACE);
    glUseProgram(ShaderProgram.ID);

    glUniform2fv(glGetUniformLocation(ShaderProgram.ID, "u_position"), 1,
                 glm::value_ptr(Entity.Position));
    glUniform2fv(glGetUniformLocation(ShaderProgram.ID, "u_size"), 1,
                 glm::value_ptr(Entity.Scale));

    Mesh_Draw(ShaderProgram.ID, Entity.Mesh);
    glEnable(GL_CULL_FACE);
}

void Renderer_DrawCubeEntity(const renderer &Renderer,
                             const shader_program &ShaderProgram,
                             const entity &Entity) {
    if (!Entity.Mesh.Material.CullFace) {
        glDisable(GL_CULL_FACE);
    }

    // 1st render pass
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    glUseProgram(ShaderProgram.ID);

    glm::vec4 Color = Entity.Mesh.Material.Color;
    glm::vec3 CubeColor = glm::vec3(Color[0], Color[1], Color[2]);
    glUniform3fv(ShaderProgram.Uniforms.EntityColorUniformLoc, 1,
                 glm::value_ptr(CubeColor));

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Entity.Position);
    Model = glm::scale(Model, Entity.Scale);

    glm::vec3 RotationVec =
        glm::vec3(Entity.Rotation[1], Entity.Rotation[2], Entity.Rotation[3]);
    Model = glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);
    glUniformMatrix4fv(ShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
                       glm::value_ptr(Model));

    Mesh_Draw(ShaderProgram.ID, Entity.Mesh);

    if (Entity.IsSelected) {
        // 2st render pass: draws the outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0xFF);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(Renderer.OutlineShaderProgram.ID);

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Entity.Position);
        Model = glm::scale(Model, glm::vec3(1.02f));
        Model =
            glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);
        glUniformMatrix4fv(
            Renderer.OutlineShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
            glm::value_ptr(Model));

        Mesh_Draw(Renderer.OutlineShaderProgram.ID, Entity.Mesh);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }

    if (!Entity.Mesh.Material.CullFace) {
        glEnable(GL_CULL_FACE);
    }
}

void Renderer_DrawModelEntity(const renderer &Renderer,
                              const shader_program &ShaderProgram,
                              const entity &Entity) {
    // 1st render pass
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    glUseProgram(ShaderProgram.ID);

    glm::vec4 Color = Entity.Mesh.Material.Color;
    glm::vec3 CubeColor = glm::vec3(Color[0], Color[1], Color[2]);
    glUniform3fv(ShaderProgram.Uniforms.EntityColorUniformLoc, 1,
                 glm::value_ptr(CubeColor));

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Entity.Position);
    Model = glm::scale(Model, Entity.Scale);

    glm::vec3 RotationVec =
        glm::vec3(Entity.Rotation[1], Entity.Rotation[2], Entity.Rotation[3]);
    Model = glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);
    glUniformMatrix4fv(ShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
                       glm::value_ptr(Model));

    Model_Draw(ShaderProgram.ID, *Entity.Model);

    if (Entity.IsSelected) {
        // 2st render pass: draws the outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0xFF);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(Renderer.OutlineShaderProgram.ID);

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Entity.Position);
        Model = glm::scale(Model, Entity.Scale + 0.01f);
        Model =
            glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);
        glUniformMatrix4fv(
            Renderer.OutlineShaderProgram.Uniforms.ModelUniformLoc, 1, GL_FALSE,
            glm::value_ptr(Model));

        Model_Draw(Renderer.OutlineShaderProgram.ID, *Entity.Model);

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
    Mesh_Draw(Renderer.SkyBoxShaderProgram.ID, SkyboxMesh);

    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}

void Renderer_SetSceneLightsUniforms(const renderer &Renderer,
                                     const shader_program &ShaderProgram,
                                     const scene &Scene, const camera &Camera) {
    glUseProgram(ShaderProgram.ID);

    // Lights
    for (size_t i = 0; i < Scene.Lights.size(); i++) {
        switch (Scene.Lights[i].LightType) {
        case light_type::Directional: {
            // Directional light
            float DirectionalLight[4] = {1.0f, -1.0f, 0.3f, 1.0f};
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
            glUniform1i(
                ShaderProgram.Uniforms.DirectionalLight.EnabledUniformLoc,
                Scene.Lights[i].IsEnabled ? 1 : 0);
            glUniform1i(
                ShaderProgram.Uniforms.DirectionalLight.UseBlinnUniformLoc,
                Scene.Lights[i].UseBlinn ? 1 : 0);
            glUniform1i(
                ShaderProgram.Uniforms.DirectionalLight.CastsShadowUniformLoc,
                Scene.Lights[i].CastsShadow ? 1 : 0);
            break;
        }
        case light_type::Point: {
            glm::vec3 PointLightAmbient = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 PointLightDiffuse =
                glm::vec3(Scene.Lights[i].Color.r, Scene.Lights[i].Color.g,
                          Scene.Lights[i].Color.b);
            glm::vec3 PointLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
            glUniform3fv(
                ShaderProgram.Uniforms.PointLights[i].PositionUniformLoc, 1,
                glm::value_ptr(Scene.Lights[i].Entity.Position));
            glUniform3fv(
                ShaderProgram.Uniforms.PointLights[i].AmbientUniformLoc, 1,
                glm::value_ptr(PointLightAmbient));
            glUniform3fv(
                ShaderProgram.Uniforms.PointLights[i].DiffuseUniformLoc, 1,
                glm::value_ptr(PointLightDiffuse));
            glUniform3fv(
                ShaderProgram.Uniforms.PointLights[i].SpecularUniformLoc, 1,
                glm::value_ptr(PointLightSpecular));
            glUniform1i(ShaderProgram.Uniforms.PointLights[i].EnabledUniformLoc,
                        Scene.Lights[i].IsEnabled ? 1 : 0);
            glUniform1i(
                ShaderProgram.Uniforms.PointLights[i].UseBlinnUniformLoc,
                Scene.Lights[i].UseBlinn ? 1 : 0);
            glUniform1i(
                ShaderProgram.Uniforms.PointLights[i].CastsShadowUniformLoc,
                Scene.Lights[i].CastsShadow ? 1 : 0);

            float Constant = 0.0f;
            float Linear = 0.0f;
            float Quadratic = 1.0f;
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
            glUniform1i(ShaderProgram.Uniforms.SpotLight.EnabledUniformLoc,
                        Scene.Lights[i].IsEnabled ? 1 : 0);
            glUniform1i(ShaderProgram.Uniforms.SpotLight.UseBlinnUniformLoc,
                        Scene.Lights[i].UseBlinn ? 1 : 0);
            glUniform1i(ShaderProgram.Uniforms.SpotLight.CastsShadowUniformLoc,
                        Scene.Lights[i].CastsShadow ? 1 : 0);

            float Constant = 0.0f;
            float Linear = 0.0f;
            float Quadratic = 1.0f;
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

void Renderer_SetOtherUniforms(const renderer &Renderer,
                               const context &Context) {
    glUseProgram(Renderer.WaterShaderProgram.ID);
    glUniform1f(Renderer.WaterShaderProgram.Uniforms.TimeUniformLoc,
                Context.LastFrame);
    glUseProgram(Renderer.ShaderProgram.ID);
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

    // Set view & projection for unlit shader
    glUseProgram(Renderer.UnlitShaderProgram.ID);
    // Sets the uniform value
    glUniformMatrix4fv(Renderer.UnlitShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(
        Renderer.UnlitShaderProgram.Uniforms.ProjectionUniformLoc, 1, GL_FALSE,
        glm::value_ptr(Projection));

    glUniform3fv(Renderer.UnlitShaderProgram.Uniforms.ViewPositionUniformLoc, 1,
                 glm::value_ptr(Camera.Position));

    // Set view & projection for unlit shader
    glUseProgram(Renderer.WaterShaderProgram.ID);
    // Sets the uniform value
    glUniformMatrix4fv(Renderer.WaterShaderProgram.Uniforms.ViewUniformLoc, 1,
                       GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(
        Renderer.WaterShaderProgram.Uniforms.ProjectionUniformLoc, 1, GL_FALSE,
        glm::value_ptr(Projection));

    glUniform3fv(Renderer.WaterShaderProgram.Uniforms.ViewPositionUniformLoc, 1,
                 glm::value_ptr(Camera.Position));

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
