# Renderer Architecture Refactor Ideas

The current renderer owns too many responsibilities: shader programs, uniform locations, framebuffer setup, OpenGL state, render pass sequencing, material selection, entity drawing, post-processing, GUI rendering, shadows, water, and skybox rendering.

This document collects ideas for improving maintainability and extensibility over time.

## 1. Introduce a `Shader` abstraction

Currently `shader_program` contains one large `uniform_locators` struct with uniforms for every possible shader. This does not scale well.

A better direction is a smaller shader wrapper:

```cpp
struct shader {
    GLuint id;
    std::unordered_map<std::string, GLint> uniforms;
};

void Shader_Use(const shader& Shader);
GLint Shader_GetUniform(shader& Shader, const char* Name);

void Shader_SetMat4(shader& Shader, const char* Name, const glm::mat4& Value);
void Shader_SetVec3(shader& Shader, const char* Name, const glm::vec3& Value);
void Shader_SetFloat(shader& Shader, const char* Name, float Value);
void Shader_SetInt(shader& Shader, const char* Name, int Value);
```

Instead of:

```cpp
glUniformMatrix4fv(ShaderProgram.Uniforms.ModelUniformLoc, ...);
```

Use:

```cpp
Shader_SetMat4(shader, "u_model", model);
```

The implementation can cache uniform locations internally, so `glGetUniformLocation` is only called once per uniform.

Benefits:

- removes the giant `uniform_locators` struct
- keeps shader-specific uniforms local to the shader
- makes adding new shaders easier
- avoids needing to add every possible uniform to every shader program struct

## 2. Replace hardcoded shader members with a shader library

The renderer currently has many direct shader members:

```cpp
shader_program ShaderProgram;
shader_program OutlineShaderProgram;
shader_program QuadShaderProgram;
shader_program ScreenShaderProgram;
shader_program SkyBoxShaderProgram;
shader_program InstanceShaderProgram;
shader_program UnlitShaderProgram;
shader_program SimpleDepthShaderProgram;
shader_program CubemapDepthShaderProgram;
shader_program BlurShaderProgram;
shader_program WaterShaderProgram;
shader_program GuiShaderProgram;
```

This grows every time a new shader is added.

Instead, introduce a shader library:

```cpp
enum class shader_type {
    Lit,
    Unlit,
    Water,
    Outline,
    Skybox,
    Screen,
    Blur,
    Depth,
    CubemapDepth,
    Gui,
    Instance
};

struct shader_library {
    std::unordered_map<shader_type, shader> shaders;
};

shader& ShaderLibrary_Get(shader_library& Library, shader_type Type);
```

Then code can request shaders by purpose:

```cpp
shader& litShader = ShaderLibrary_Get(Renderer.Shaders, shader_type::Lit);
```

Benefits:

- renderer struct stops growing with every shader
- shader lookup becomes data-driven
- materials and passes can reference shader types instead of concrete renderer fields

## 3. Split `Renderer_Draw` into render passes

`Renderer_Draw` currently performs many independent stages:

1. directional shadow pass
2. point shadow cubemap pass
3. water refraction pass
4. water reflection pass
5. main scene pass
6. water pass
7. instancing pass
8. skybox pass
9. bloom blur pass
10. GUI pass
11. final screen/present pass

Each of these can become its own render pass:

```cpp
void Renderer_Draw(renderer& Renderer, const scene& Scene, const context& Context) {
    ShadowPass_Render(Renderer.ShadowPass, Scene, Context);
    PointShadowPass_Render(Renderer.PointShadowPass, Scene, Context);
    WaterRefractionPass_Render(Renderer.WaterPass, Scene, Context);
    WaterReflectionPass_Render(Renderer.WaterPass, Scene, Context);
    MainScenePass_Render(Renderer.MainPass, Scene, Context);
    BloomPass_Render(Renderer.BloomPass, Scene, Context);
    GuiPass_Render(Renderer.GuiPass, Scene, Context);
    PresentPass_Render(Renderer.PresentPass, Scene, Context);
}
```

Example pass structs:

```cpp
struct shadow_pass {
    GLuint fbo;
    GLuint depthMap;
    shader* depthShader;
};

struct bloom_pass {
    GLuint pingpongFBO[2];
    GLuint pingpongColorBuffers[2];
    shader* blurShader;
};

struct water_pass {
    GLuint refractionFBO;
    GLuint reflectionFBO;
    GLuint refractionColor;
    GLuint reflectionColor;
    GLuint refractionDepth;
    shader* waterShader;
};
```

Benefits:

- `Renderer_Draw` becomes an orchestrator
- individual passes become easier to test and modify
- water, bloom, shadows, GUI, and presentation can evolve independently

## 4. Add a per-frame `render_context`

Many functions currently pass around the renderer, scene, context, camera, screen sizes, and matrices separately.

Introduce a temporary per-frame context:

```cpp
struct render_context {
    const scene* Scene;
    const context* AppContext;
    const camera* Camera;

    glm::mat4 View;
    glm::mat4 Projection;
    glm::vec3 CameraPosition;

    int Width;
    int Height;

    glm::mat4 LightSpaceMatrix;
    float NearPlane;
    float FarPlane;
};
```

Render passes can then receive:

```cpp
void MainPass_Render(main_pass& Pass, renderer& Renderer, const render_context& RC);
```

Benefits:

- avoids recomputing common matrices
- reduces long parameter lists
- makes per-frame rendering state explicit

## 5. Move common uniform setup into helpers

The renderer currently has broad functions such as:

```cpp
Renderer_SetCameraUniforms(...);
Renderer_SetSceneLightsUniforms(...);
Renderer_SetOtherUniforms(...);
```

These functions still know about many specific shaders.

Prefer intent-based helpers:

```cpp
void BindCameraUniforms(shader& Shader, const render_context& RC);
void BindObjectUniforms(shader& Shader, const entity& Entity);
void BindMaterialUniforms(shader& Shader, const material& Material);
void BindLightingUniforms(shader& Shader, const scene& Scene, const camera& Camera);
void BindShadowUniforms(shader& Shader, const shadow_pass& ShadowPass);
```

Then each pass binds only what it needs:

```cpp
Shader_Use(litShader);
BindCameraUniforms(litShader, RC);
BindLightingUniforms(litShader, Scene, Camera);
BindShadowUniforms(litShader, Renderer.ShadowPass);
```

Benefits:

- reduces duplicated uniform code
- keeps each pass responsible for its own required data
- avoids one function needing to know every shader in the renderer

## 6. Use render queues instead of drawing directly from the scene

Currently `Renderer_DrawScene` loops over `Scene.Entities` and switches on material type and entity type.

A render queue can categorize visible render items once per frame:

```cpp
struct render_item {
    entity* Entity;
    mesh* Mesh;
    material* Material;
    glm::mat4 Model;
};

struct render_queue {
    std::vector<render_item> Opaque;
    std::vector<render_item> Transparent;
    std::vector<render_item> Water;
    std::vector<render_item> Gui;
    std::vector<render_item> Debug;
};
```

Build the queue once:

```cpp
RenderQueue_Build(Scene, Queue);
```

Then passes render the queues they care about:

```cpp
MainPass_RenderOpaque(Queue.Opaque);
WaterPass_Render(Queue.Water);
GuiPass_Render(Queue.Gui);
```

Benefits:

- avoids repeated filtering inside render passes
- makes transparency sorting easier later
- makes culling and batching easier to introduce
- separates scene data from renderable data

## 7. Make materials select shaders

Current shader selection is hardcoded in `Renderer_DrawScene`:

```cpp
switch (Entity.Mesh.Material.ShaderMaterial) {
case shader_material::Default:
    Shader = Renderer.ShaderProgram;
    break;
case shader_material::Unlit:
    Shader = Renderer.UnlitShaderProgram;
    break;
case shader_material::Water:
    continue;
}
```

A better material shape would be:

```cpp
struct material {
    shader_type ShaderType;
    std::vector<texture*> Textures;
    glm::vec4 Color;
    float Shininess;
    bool HasNormalMap;
    bool HasSpecularMap;
    bool CullFace;
};
```

Then shader selection becomes:

```cpp
shader& Shader = ShaderLibrary_Get(Renderer.Shaders, Material.ShaderType);
```

Benefits:

- adding a new material shader does not require editing the renderer draw loop
- material behavior becomes more data-driven
- render queue sorting/grouping by shader becomes easier

## 8. Encapsulate framebuffer objects

The renderer currently has many loose framebuffer-related fields:

```cpp
GLuint FrameBuffer;
GLuint TextureColorBuffer;
GLuint BrightColorBuffer;
GLuint RBO;
GLuint RefractionFBO;
GLuint ReflectionFBO;
// ...
```

Create reusable framebuffer structs:

```cpp
struct framebuffer {
    GLuint id;
    int width;
    int height;
    std::vector<GLuint> colorAttachments;
    GLuint depthAttachment;
    GLuint renderbuffer;
};
```

Then use helper functions:

```cpp
Framebuffer_CreateHDR(...);
Framebuffer_CreateDepthMap(...);
Framebuffer_CreateWaterReflection(...);
Framebuffer_Resize(...);
Framebuffer_Destroy(...);
Framebuffer_Bind(...);
```

Benefits:

- shrinks `Renderer_Create`
- makes resize and cleanup safer
- avoids scattering framebuffer setup details throughout the renderer

## Suggested refactor order

Do not try to do everything at once. A safe order would be:

1. Add `Shader` helper functions while keeping the current renderer structure.
2. Replace direct uniform location structs with cached string-based uniform lookup.
3. Create framebuffer wrapper structs.
4. Extract render passes from `Renderer_Draw` one by one.
5. Introduce `shader_library`.
6. Introduce `render_queue`.
7. Move toward material-driven shader selection.

The highest-impact first step is extracting the major render passes:

```cpp
ShadowPass_Render(...);
WaterPass_Render(...);
MainPass_Render(...);
BloomPass_Render(...);
PresentPass_Render(...);
```

This alone would make `renderer.cpp` much easier to navigate and maintain.
