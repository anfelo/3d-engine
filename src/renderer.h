#ifndef RENDERER_H_
#define RENDERER_H_

#include "camera.h"
#include "context.h"
#include "resource_manager.h"
#include "scene.h"
#include "shader.h"
#include "texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

struct renderer {
    resource_manager ResourceManager;

    // Framebuffer stuff
    GLuint FrameBufferVAO, FrameBufferVBO;
    GLuint FrameBuffer;
    GLuint TextureColorBuffer;
    GLuint BrightColorBuffer;
    GLuint RBO; // render buffer object

    // PingPong Framebuffers stuff (for bloom filter)
    GLuint PingPongFBO[2];
    GLuint PingPongColorBuffers[2];
    bool CurrentPingPongBuffer;

    // Depth Map stuff
    GLuint DepthMapFBO;
    GLuint DepthMapBuffer;

    // Depth Cubemap stuff
    GLuint DepthCubemapFBO;
    GLuint DepthCubemapBuffer;

    // Water Framebuffers stuff
    GLuint RefractionFBO;
    GLuint RefractionDepthBuffer;
    GLuint RefractionColorBuffer;
    GLuint RefractionRBO;
    int RefractionFBOWidth;
    int RefractionFBOHeight;

    GLuint ReflectionFBO;
    GLuint ReflectionDepthBuffer;
    GLuint ReflectionColorBuffer;
    GLuint ReflectionRBO;
    int ReflectionFBOWidth;
    int ReflectionFBOHeight;
};

renderer Renderer_Create(const context &Context);
void Renderer_Destroy(renderer &Renderer);
void Renderer_ResizeFramebuffer(const renderer &Renderer, int ScreenWidth,
                                int ScreenHeight);
void Renderer_BindFramebuffer(const renderer &Renderer, GLuint FramebufferID,
                              int Width, int Height);
void Renderer_DirectionalShadowPass(const renderer &Renderer,
                                    const scene &Scene, const context &Context);
void Renderer_PointShadowPass(const renderer &Renderer, const scene &Scene,
                              const context &Context);
void Renderer_WaterRefractionPass(const renderer &Renderer, const scene &Scene,
                                  const context &Context);
void Renderer_WaterReflectionPass(const renderer &Renderer, const scene &Scene,
                                  const context &Context);
void Renderer_BloomPass(renderer &Renderer, const scene &Scene,
                        const context &Context);
void Renderer_MainScenePass(const renderer &Renderer, const scene &Scene,
                            const context &Context);
void Renderer_GuiPass(const renderer &Renderer, const scene &Scene,
                      const context &Context);
void Renderer_PresentPass(const renderer &Renderer, const scene &Scene,
                          bool PingPongBuffer, const context &Context);
void Renderer_Draw(renderer &Renderer, const scene &Scene,
                   const context &Context);
void Renderer_DrawScene(const renderer &Renderer, const shader &ShaderProgram,
                        const scene &Scene, bool useEntityShader = true);
void Renderer_DrawSceneWater(const renderer &Renderer, const scene &Scene);
void Renderer_DrawQuadEntity(const renderer &Renderer,
                             const shader &ShaderProgram, const entity &Entity);
void Renderer_DrawCubeEntity(const renderer &Renderer,
                             const shader &ShaderProgram, const entity &Entity);
void Renderer_DrawModelEntity(const renderer &Renderer,
                              const shader &ShaderProgram,
                              const entity &Entity);
void Renderer_DrawGuiEntity(const renderer &Renderer,
                            const shader &ShaderProgram, const entity &Entity);
void Renderer_DrawSkybox(const renderer &Renderer, const skybox &Skybox);
void Renderer_SetSceneLightsUniforms(const renderer &Renderer,
                                     const shader &ShaderProgram,
                                     const scene &Scene, const camera &Camera);
void Renderer_ClearBackground(float R, float G, float B, float Alpha);
void Renderer_SetCameraUniforms(const renderer &Renderer, const camera &Camera,
                                float ScreenWidth, float ScreenHeight);
void Renderer_SetShaderCameraUniforms(const renderer &Renderer,
                                      const shader &Shader, glm::mat4 View,
                                      glm::vec3 ViewPosition,
                                      glm::mat4 Projection);
void Renderer_SetOtherUniforms(const renderer &Renderer,
                               const context &Context);
void Renderer_SetTextureUniforms(const renderer &Renderer);

#endif
