#include "renderer.h"

#include <string>
#include <iostream>
#include <cerrno>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "entity.h"
#include "material.h"
#include "mesh.h"
#include "model.h"
#include "resource_manager.h"
#include "scene.h"
#include "shader.h"

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
    ResourceManager_ClearResources(Renderer.ResourceManager);

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

        const shader *WaterShader = ResourceManager_GetShader(
            Renderer.ResourceManager, shader_type::Water);
        Renderer_DrawQuadEntity(Renderer, *WaterShader, Entity);
    }
}

void Renderer_DrawScene(const renderer &Renderer, const shader &Shader,
                        const scene &Scene, bool useEntityShader) {

    // Entities
    for (entity Entity : Scene.Entities) {
        shader _Shader = Shader;
        if (useEntityShader) {
            switch (Entity.Mesh.Material.ShaderMaterial) {
            case shader_material::Default: {
                const shader *DefaultShader = ResourceManager_GetShader(
                    Renderer.ResourceManager, shader_type::Lit);
                _Shader = *DefaultShader;
                break;
            }
            case shader_material::Unlit: {
                const shader *UnlitShader = ResourceManager_GetShader(
                    Renderer.ResourceManager, shader_type::Unlit);
                _Shader = *UnlitShader;
                break;
            }
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
            Renderer_DrawCubeEntity(Renderer, _Shader, Entity);
            break;
        case entity_type::Model:
            Renderer_DrawModelEntity(Renderer, _Shader, Entity);
            break;
        case entity_type::Triangle:
            // TODO: Make sure this works
            Renderer_DrawTriangle(Renderer, _Shader, Entity.Position);
            break;
        case entity_type::Quad:
            // TODO: Merge this with the QuadMesh
            break;
        case entity_type::QuadMesh:
            Renderer_DrawQuadEntity(Renderer, _Shader, Entity);
            break;
        }
    }

    if (useEntityShader) {
        // Lights (Debug)
        for (light Light : Scene.Lights) {
            if (Light.ShowDebug && Light.LightType == light_type::Point) {
                shader _Shader = Shader;
                switch (Light.Entity.Mesh.Material.ShaderMaterial) {
                case shader_material::Default: {
                    const shader *DefaultShader = ResourceManager_GetShader(
                        Renderer.ResourceManager, shader_type::Lit);
                    _Shader = *DefaultShader;
                } break;
                case shader_material::Unlit: {
                    const shader *UnlitShader = ResourceManager_GetShader(
                        Renderer.ResourceManager, shader_type::Unlit);
                    _Shader = *UnlitShader;
                    break;
                }
                default:
                    break;
                }

                Renderer_DrawCubeEntity(Renderer, _Shader, Light.Entity);
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

    const shader *DepthShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Depth);
    Shader_SetMat4(*DepthShader, "u_light_space_matrix", LightSpaceMatrix);

    // Remove peter panning problems
    // glCullFace(GL_FRONT);
    Renderer_DrawScene(Renderer, *DepthShader, Scene, false);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Point Shadow Mapping
    glViewport(0, 0, Context.ShadowbufferWidth, Context.ShadowbufferHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, Renderer.DepthCubemapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set view & projection for depth shader from the light's perspective
    float Aspect =
        (float)Context.ShadowbufferWidth / (float)Context.ShadowbufferHeight;
    NearPlane = 0.1f;
    FarPlane = 25.0f;

    bool HasPointLight = false;
    glm::vec3 PointLightPosition(0.0f);
    for (const light &Light : Scene.Lights) {
        if (Light.LightType == light_type::Point) {
            HasPointLight = true;
            PointLightPosition = Light.Entity.Position;
            break;
        }
    }

    if (HasPointLight) {
        glm::mat4 PointLightProjection =
            glm::perspective(glm::radians(90.0f), Aspect, NearPlane, FarPlane);

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

        const shader *CubemapDepthShader = ResourceManager_GetShader(
            Renderer.ResourceManager, shader_type::CubemapDepth);
        Shader_SetVec3(*CubemapDepthShader, "u_light_pos", PointLightPosition);
        Shader_SetFloat(*CubemapDepthShader, "u_far_plane", FarPlane);
        for (unsigned int i = 0; i < 6; ++i) {
            std::string UniformName =
                ("u_shadow_matrices[" + std::to_string(i) + "]");
            Shader_SetMat4(*CubemapDepthShader, UniformName.c_str(),
                           PointShadowTransforms[i]);
        }

        Renderer_DrawScene(Renderer, *CubemapDepthShader, Scene, false);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);
    Renderer_BindFramebuffer(Renderer, Renderer.RefractionFBO,
                             Renderer.RefractionFBOWidth,
                             Renderer.RefractionFBOHeight);
    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const shader *LitShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Lit);
    const shader *UnlitShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Unlit);
    const shader *InstanceShader = ResourceManager_GetShader(
        Renderer.ResourceManager, shader_type::Instance);
    const shader *WaterShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Water);

    glm::vec4 RefractionClipPlane = glm::vec4(0.0f, -1.0f, 0.0f, 0.01f);
    Shader_SetVec4(*LitShader, "u_clip_plane", RefractionClipPlane);
    Shader_SetVec4(*UnlitShader, "u_clip_plane", RefractionClipPlane);
    Shader_SetVec4(*InstanceShader, "u_clip_plane", RefractionClipPlane);
    Shader_SetVec4(*WaterShader, "u_clip_plane", RefractionClipPlane);

    Renderer_SetCameraUniforms(Renderer, Context.Camera, Context.ScreenWidth,
                               Context.ScreenHeight);
    Renderer_SetSceneLightsUniforms(Renderer, *LitShader, Scene,
                                    Context.Camera);
    Renderer_SetOtherUniforms(Renderer, Context);

    Renderer_DrawScene(Renderer, *LitShader, Scene);
    Renderer_DrawSkybox(Renderer, Scene.Skybox);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);

    Renderer_BindFramebuffer(Renderer, Renderer.ReflectionFBO,
                             Renderer.ReflectionFBOWidth,
                             Renderer.ReflectionFBOHeight);
    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec4 ReflectionClipPlane = glm::vec4(0.0f, 1.0f, 0.0f, -0.01f);
    Shader_SetVec4(*LitShader, "u_clip_plane", ReflectionClipPlane);
    Shader_SetVec4(*UnlitShader, "u_clip_plane", ReflectionClipPlane);
    Shader_SetVec4(*InstanceShader, "u_clip_plane", ReflectionClipPlane);
    Shader_SetVec4(*WaterShader, "u_clip_plane", ReflectionClipPlane);

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
    Renderer_SetSceneLightsUniforms(Renderer, *LitShader, Scene,
                                    Context.Camera);
    Renderer_SetOtherUniforms(Renderer, Context);

    Renderer_DrawScene(Renderer, *LitShader, Scene);
    Renderer_DrawSkybox(Renderer, Scene.Skybox);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Context.FramebufferWidth, Context.FramebufferHeight);

    // 2st. Pass: Draw the scene to the framebuffer
    // bind to framebuffer and draw scene as we normally would to color
    // texture
    Renderer_BindFramebuffer(Renderer, Renderer.FrameBuffer,
                             Context.FramebufferWidth,
                             Context.FramebufferHeight);

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
    Shader_SetVec4(*LitShader, "u_clip_plane", NoClipPlane);
    Shader_SetVec4(*UnlitShader, "u_clip_plane", NoClipPlane);
    Shader_SetVec4(*InstanceShader, "u_clip_plane", NoClipPlane);
    Shader_SetVec4(*WaterShader, "u_clip_plane", NoClipPlane);

    glUseProgram(LitShader->ID);

    // Sets the view & projection uniforms for all the programs
    Renderer_SetCameraUniforms(Renderer, Context.Camera, Context.ScreenWidth,
                               Context.ScreenHeight);

    Renderer_SetSceneLightsUniforms(Renderer, *LitShader, Scene,
                                    Context.Camera);

    Renderer_SetOtherUniforms(Renderer, Context);

    // Directional Shadow Map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Renderer.DepthMapBuffer);
    Shader_SetInt(*LitShader, "u_shadow_map", 3);

    // Point Shadow Cubemap
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Renderer.DepthCubemapBuffer);
    Shader_SetInt(*LitShader, "u_shadow_cubemap", 4);

    Shader_SetMat4(*LitShader, "u_light_space_matrix", LightSpaceMatrix);
    Shader_SetFloat(*LitShader, "u_far_plane", FarPlane);

    Renderer_DrawScene(Renderer, *LitShader, Scene);

    // TODO: Refactor the Water Renderer
    glUseProgram(WaterShader->ID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, Renderer.RefractionColorBuffer);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Renderer.ReflectionColorBuffer);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Renderer.RefractionDepthBuffer);

    NearPlane = 0.1f;
    FarPlane = 1000.0f;
    Shader_SetFloat(*WaterShader, "u_near_plane", NearPlane);
    Shader_SetFloat(*WaterShader, "u_far_plane", FarPlane);
    Renderer_SetSceneLightsUniforms(Renderer, *WaterShader, Scene,
                                    Context.Camera);
    glUseProgram(LitShader->ID);
    Renderer_DrawSceneWater(Renderer, Scene);

    // TODO: Keeping the instances out of the shadow pass for now.
    if (Scene.Instances.size() > 0) {
        glUseProgram(InstanceShader->ID);

        Renderer_SetSceneLightsUniforms(Renderer, *InstanceShader, Scene,
                                        Context.Camera);

        // Directional Shadow Map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, Renderer.DepthMapBuffer);
        Shader_SetInt(*InstanceShader, "u_shadow_map", 3);

        // Point Shadow Cubemap
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Renderer.DepthCubemapBuffer);
        Shader_SetInt(*InstanceShader, "u_shadow_cubemap", 4);
        Shader_SetMat4(*InstanceShader, "u_light_space_matrix",
                       LightSpaceMatrix);
        Shader_SetFloat(*InstanceShader, "u_far_plane", FarPlane);

        model *Model = Scene.Instances[0].Model;
        Model_DrawInstances(*Model, *InstanceShader, Scene.Instances.size());
    }

    // Skybox
    Renderer_DrawSkybox(Renderer, Scene.Skybox);

    // Bloom Blur Pass: blur bright fragments with two-pass Gaussian Blur
    // --------------------------------------------------
    bool Horizontal = true, FirstIteration = true;
    unsigned int Amount = 10;
    const shader *BlurShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Blur);
    glUseProgram(BlurShader->ID);
    glBindVertexArray(Renderer.FrameBufferVAO);
    glActiveTexture(GL_TEXTURE0);
    for (unsigned int i = 0; i < Amount; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, Renderer.PingPongFBO[Horizontal]);
        Shader_SetInt(*BlurShader, "u_horizontal", Horizontal);
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
    const shader *GuiShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Gui);
    glUseProgram(GuiShader->ID);
    glm::vec2 ScreenSize =
        glm::vec2(Context.FramebufferWidth, Context.FramebufferHeight);
    Shader_SetVec2(*GuiShader, "u_screen_size", ScreenSize);
    for (unsigned int i = 0; i < Scene.GuiTextures.size(); ++i) {
        Renderer_DrawGuiEntity(Renderer, *GuiShader, Scene.GuiTextures.at(i));
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

    const shader *ScreenShader = ResourceManager_GetShader(
        Renderer.ResourceManager, shader_type::Screen);
    glUseProgram(ScreenShader->ID);
    glBindVertexArray(Renderer.FrameBufferVAO);

    Shader_SetInt(*ScreenShader, "u_effect", Scene.Effect);
    Shader_SetInt(*ScreenShader, "u_hdr_enabled", Scene.HDREnabled ? 1 : 0);
    Shader_SetFloat(*ScreenShader, "u_exposure", Scene.HDRExposure);
    Shader_SetInt(*ScreenShader, "u_bloom_enabled", Scene.BloomEnabled ? 1 : 0);

    // use the color attachment texture as
    // the texture of the quad plane
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Renderer.TextureColorBuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Renderer.PingPongColorBuffers[!Horizontal]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// TODO: Needs fixing
void Renderer_DrawTriangle(const renderer &Renderer, const shader &Shader,
                           glm::vec<3, float> position) {
    Shader_Use(Shader);

    glm::mat4 View = glm::mat4(1.0f);
    View = glm::translate(View, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 Projection;
    Projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Sets the uniform value
    Shader_SetMat4(Shader, "u_view", View);
    Shader_SetMat4(Shader, "u_projection", Projection);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, position);
    Shader_SetMat4(Shader, "u_model", Model);

    // glBindVertexArray(TriangleMesh.VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer_DrawQuadEntity(const renderer &Renderer, const shader &Shader,
                             const entity &Entity) {
    glDisable(GL_CULL_FACE);
    Shader_Use(Shader);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Entity.Position);
    Model = glm::scale(Model, Entity.Scale);

    glm::vec3 RotationVec =
        glm::vec3(Entity.Rotation[1], Entity.Rotation[2], Entity.Rotation[3]);
    Model = glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);

    glm::vec4 Color = Entity.Mesh.Material.Color;
    glm::vec3 QuadColor = glm::vec3(Color[0], Color[1], Color[2]);
    Shader_SetVec3(Shader, "u_entity_color", QuadColor);
    Shader_SetMat4(Shader, "u_model", Model);

    Mesh_Draw(Entity.Mesh, Shader);
    glEnable(GL_CULL_FACE);
}

void Renderer_DrawGuiEntity(const renderer &Renderer, const shader &Shader,
                            const entity &Entity) {
    glDisable(GL_CULL_FACE);
    Shader_Use(Shader);

    Shader_SetVec2(Shader, "u_position", Entity.Position);
    Shader_SetVec2(Shader, "u_size", Entity.Scale);

    Mesh_Draw(Entity.Mesh, Shader);
    glEnable(GL_CULL_FACE);
}

void Renderer_DrawCubeEntity(const renderer &Renderer, const shader &Shader,
                             const entity &Entity) {
    if (!Entity.Mesh.Material.CullFace) {
        glDisable(GL_CULL_FACE);
    }

    // 1st render pass
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    Shader_Use(Shader);

    glm::vec4 Color = Entity.Mesh.Material.Color;
    glm::vec3 CubeColor = glm::vec3(Color[0], Color[1], Color[2]);
    Shader_SetVec3(Shader, "u_entity_color", CubeColor);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Entity.Position);
    Model = glm::scale(Model, Entity.Scale);

    glm::vec3 RotationVec =
        glm::vec3(Entity.Rotation[1], Entity.Rotation[2], Entity.Rotation[3]);
    Model = glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);
    Shader_SetMat4(Shader, "u_model", Model);

    Mesh_Draw(Entity.Mesh, Shader);

    if (Entity.IsSelected) {
        // 2st render pass: draws the outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0xFF);
        glDisable(GL_DEPTH_TEST);

        const shader *OutlineShader = ResourceManager_GetShader(
            Renderer.ResourceManager, shader_type::Outline);
        Shader_Use(*OutlineShader);

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Entity.Position);
        Model = glm::scale(Model, glm::vec3(1.02f));
        Model =
            glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);
        Shader_SetMat4(*OutlineShader, "u_model", Model);

        Mesh_Draw(Entity.Mesh, *OutlineShader);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }

    if (!Entity.Mesh.Material.CullFace) {
        glEnable(GL_CULL_FACE);
    }
}

void Renderer_DrawModelEntity(const renderer &Renderer, const shader &Shader,
                              const entity &Entity) {
    // 1st render pass
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    Shader_Use(Shader);

    glm::vec4 Color = Entity.Mesh.Material.Color;
    glm::vec3 CubeColor = glm::vec3(Color[0], Color[1], Color[2]);
    Shader_SetVec3(Shader, "u_entity_color", CubeColor);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Entity.Position);
    Model = glm::scale(Model, Entity.Scale);

    glm::vec3 RotationVec =
        glm::vec3(Entity.Rotation[1], Entity.Rotation[2], Entity.Rotation[3]);
    Model = glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);
    Shader_SetMat4(Shader, "u_model", Model);

    Model_Draw(*Entity.Model, Shader);

    if (Entity.IsSelected) {
        // 2st render pass: draws the outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0xFF);
        glDisable(GL_DEPTH_TEST);

        const shader *OutlineShader = ResourceManager_GetShader(
            Renderer.ResourceManager, shader_type::Outline);
        Shader_Use(*OutlineShader);

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Entity.Position);
        Model = glm::scale(Model, Entity.Scale + 0.01f);
        Model =
            glm::rotate(Model, glm::radians(Entity.Rotation[0]), RotationVec);
        Shader_SetMat4(*OutlineShader, "u_model", Model);

        Model_Draw(*Entity.Model, *OutlineShader);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }
}

void Renderer_DrawSkybox(const renderer &Renderer, const skybox &Skybox) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    const shader *SkyboxShader = ResourceManager_GetShader(
        Renderer.ResourceManager, shader_type::Skybox);
    Shader_Use(*SkyboxShader);

    mesh SkyboxMesh = Skybox.Mesh;
    Mesh_Draw(SkyboxMesh, *SkyboxShader);

    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}

void Renderer_SetSceneLightsUniforms(const renderer &Renderer,
                                     const shader &Shader, const scene &Scene,
                                     const camera &Camera) {
    Shader_Use(Shader);

    // Lights
    char Buffer[100];
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

            Shader_SetVec3(Shader, "u_dir_light.direction",
                           DirectionalLightDir);
            Shader_SetVec3(Shader, "u_dir_light.ambient",
                           DirectionalLightAmbient);
            Shader_SetVec3(Shader, "u_dir_light.diffuse",
                           DirectionalLightDiffuse);
            Shader_SetVec3(Shader, "u_dir_light.specular",
                           DirectionalLightSpecular);
            Shader_SetInt(Shader, "u_dir_light.enabled",
                          Scene.Lights[i].IsEnabled ? 1 : 0);
            Shader_SetInt(Shader, "u_dir_light.blinn",
                          Scene.Lights[i].UseBlinn ? 1 : 0);
            Shader_SetInt(Shader, "u_dir_light.casts_shadow",
                          Scene.Lights[i].CastsShadow ? 1 : 0);
            break;
        }
        case light_type::Point: {
            glm::vec3 PointLightAmbient = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 PointLightDiffuse =
                glm::vec3(Scene.Lights[i].Color.r, Scene.Lights[i].Color.g,
                          Scene.Lights[i].Color.b);
            glm::vec3 PointLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

            int I = (int)i;
            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].position", I);
            Shader_SetVec3(Shader, Buffer, Scene.Lights[i].Entity.Position);

            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].ambient", I);
            Shader_SetVec3(Shader, Buffer, PointLightAmbient);

            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].diffuse", I);
            Shader_SetVec3(Shader, Buffer, PointLightDiffuse);

            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].specular", I);
            Shader_SetVec3(Shader, Buffer, PointLightSpecular);

            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].enabled", I);
            Shader_SetInt(Shader, Buffer, Scene.Lights[i].IsEnabled ? 1 : 0);

            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].blinn", I);
            Shader_SetInt(Shader, Buffer, Scene.Lights[i].UseBlinn ? 1 : 0);

            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].casts_shadow",
                     I);
            Shader_SetInt(Shader, Buffer, Scene.Lights[i].CastsShadow ? 1 : 0);

            float Constant = 0.0f;
            float Linear = 0.0f;
            float Quadratic = 1.0f;
            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].constant", I);
            Shader_SetFloat(Shader, Buffer, Constant);

            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].linear", I);
            Shader_SetFloat(Shader, Buffer, Linear);

            snprintf(Buffer, sizeof(Buffer), "u_point_lights[%d].quadratic", I);
            Shader_SetFloat(Shader, Buffer, Quadratic);
            break;
        }
        case light_type::Spot: {
            glm::vec3 SpotLightAmbient = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 SpotLightDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 SpotLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
            Shader_SetVec3(Shader, "u_spot_light.position", Camera.Position);
            Shader_SetVec3(Shader, "u_spot_light.direction", Camera.Front);
            Shader_SetVec3(Shader, "u_spot_light.ambient", SpotLightAmbient);
            Shader_SetVec3(Shader, "u_spot_light.diffuse", SpotLightDiffuse);
            Shader_SetVec3(Shader, "u_spot_light.specular", SpotLightSpecular);
            Shader_SetInt(Shader, "u_spot_light.enabled",
                          Scene.Lights[i].IsEnabled ? 1 : 0);
            Shader_SetInt(Shader, "u_spot_light.blinn",
                          Scene.Lights[i].UseBlinn ? 1 : 0);
            Shader_SetInt(Shader, "u_spot_light.casts_shadow",
                          Scene.Lights[i].CastsShadow ? 1 : 0);

            float Constant = 0.0f;
            float Linear = 0.0f;
            float Quadratic = 1.0f;
            float CutOff = glm::cos(glm::radians(12.5f));
            float OuterCutOff = glm::cos(glm::radians(15.0f));
            Shader_SetFloat(Shader, "u_spot_light.constant", Constant);
            Shader_SetFloat(Shader, "u_spot_light.linear", Linear);
            Shader_SetFloat(Shader, "u_spot_light.quadratic", Quadratic);
            Shader_SetFloat(Shader, "u_spot_light.cut_off", CutOff);
            Shader_SetFloat(Shader, "u_spot_light.outer_cut_off", OuterCutOff);
            break;
        }
        }
    }
}

void Renderer_SetOtherUniforms(const renderer &Renderer,
                               const context &Context) {
    const shader *WaterShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Water);
    const shader *LitShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Lit);
    Shader_SetFloat(*WaterShader, "u_time", Context.LastFrame);
    Shader_Use(*LitShader);
}

void Renderer_SetTextureUniforms(const renderer &Renderer) {
    const shader *BlurShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Blur);
    const shader *WaterShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Water);
    const shader *ScreenShader = ResourceManager_GetShader(
        Renderer.ResourceManager, shader_type::Screen);

    Shader_SetInt(*BlurShader, "u_screen_texture", 0);
    Shader_SetInt(*WaterShader, "u_refraction_texture", 2);
    Shader_SetInt(*WaterShader, "u_reflection_texture", 3);
    Shader_SetInt(*WaterShader, "u_depth_map", 4);
    Shader_SetInt(*ScreenShader, "u_screen_texture", 0);
    Shader_SetInt(*ScreenShader, "u_bloom_texture", 1);
}

void Renderer_SetShaderCameraUniforms(const renderer &Renderer,
                                      const shader &Shader, glm::mat4 View,
                                      glm::vec3 ViewPosition,
                                      glm::mat4 Projection) {
    Shader_SetMat4(Shader, "u_view", View);
    Shader_SetMat4(Shader, "u_projection", Projection);
    Shader_SetVec3(Shader, "u_view_pos", ViewPosition);
}

void Renderer_SetCameraUniforms(const renderer &Renderer, const camera &Camera,
                                float ScreenWidth, float ScreenHeight) {
    glm::mat4 View = Camera_GetViewMatrix(Camera);
    glm::mat4 Projection = glm::perspective(
        glm::radians(Camera.Zoom), ScreenWidth / ScreenHeight, 0.1f, 100.0f);

    const shader *LitShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Lit);
    const shader *OutlineShader = ResourceManager_GetShader(
        Renderer.ResourceManager, shader_type::Outline);
    const shader *QuadShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Quad);
    const shader *UnlitShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Unlit);
    const shader *WaterShader =
        ResourceManager_GetShader(Renderer.ResourceManager, shader_type::Water);
    const shader *SkyboxShader = ResourceManager_GetShader(
        Renderer.ResourceManager, shader_type::Skybox);
    const shader *InstanceShader = ResourceManager_GetShader(
        Renderer.ResourceManager, shader_type::Instance);

    Renderer_SetShaderCameraUniforms(Renderer, *LitShader, View,
                                     Camera.Position, Projection);
    Renderer_SetShaderCameraUniforms(Renderer, *OutlineShader, View,
                                     Camera.Position, Projection);
    Renderer_SetShaderCameraUniforms(Renderer, *QuadShader, View,
                                     Camera.Position, Projection);
    Renderer_SetShaderCameraUniforms(Renderer, *InstanceShader, View,
                                     Camera.Position, Projection);
    Renderer_SetShaderCameraUniforms(Renderer, *UnlitShader, View,
                                     Camera.Position, Projection);
    Renderer_SetShaderCameraUniforms(Renderer, *WaterShader, View,
                                     Camera.Position, Projection);
    // Set view & projection for skybox shader
    View = glm::mat4(glm::mat3(View));
    Renderer_SetShaderCameraUniforms(Renderer, *SkyboxShader, View,
                                     Camera.Position, Projection);
}
