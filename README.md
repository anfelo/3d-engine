# 3D Engine

A from-scratch 3D engine written in C++ and modern OpenGL, built as a learning project to understand graphics programming and engine architecture in depth.

It focuses on practical rendering fundamentals: scene composition, camera movement, model loading, lighting, transparency, and large-instance rendering.

## Screenshot

<img width="1247" height="873" alt="3D Engine Screenshot" src="https://github.com/user-attachments/assets/aaf6ad0d-3460-4546-9ee6-cc721801bfe2" />

## Current Features

- Scene/entity setup with reusable mesh and model data.
- Free-fly camera with keyboard movement and mouse look.
- Mesh and model rendering with transforms (translate/rotate/scale).
- Texture workflows including diffuse, specular, and normal maps.
- Multiple light types (directional, point, and spotlight).
- Transparency rendering for quads (grass/windows style surfaces).
- Skybox cubemap rendering.
- Instanced rendering support for large asteroid fields.
- Runtime scene tooling with Dear ImGui.

## Tech Stack

- C++17
- OpenGL 3.3 Core Profile
- GLFW (windowing/input)
- GLAD (OpenGL loader)
- Assimp (model importing)
- GLM (math)
- stb_image (texture loading)
- Dear ImGui (debug/editor UI)

## Platform Support

- Tested: macOS
- Linux: intended via Makefile targets/dependencies, currently unverified
- Windows: intended but currently unverified

## Prerequisites

Install or provide the following on your system:

- A C++17-capable compiler (`clang++` or `g++`)
- OpenGL 3.3 compatible GPU/driver
- GLFW development libraries
- Assimp development libraries

Project-local sources already include GLAD, Dear ImGui, and stb_image.

## Build and Run

From the project root:

```bash
make
./bin/3DEngine
```

The build outputs:

- Executable: `bin/3DEngine`
- Runtime assets copied to: `bin/resources`

Clean build artifacts:

```bash
make clean
```

## Controls

- `W` / `A` / `S` / `D`: move forward / left / back / right
- `Space`: move up
- `Left Ctrl`: move down
- Hold `Left Mouse Button`: enable mouse-look camera rotation
- Mouse wheel: zoom in/out (camera FOV)
- `Esc`: close the application

## Project Structure

- `src/main.cpp` - application setup, scene bootstrap, main loop, and input handling
- `src/renderer.*` - render pipeline and scene drawing
- `src/scene.*` - scene state, entities, lights, and instances
- `src/model.*`, `src/mesh.*`, `src/texture.*` - asset and geometry rendering primitives
- `src/camera.*` - camera math and movement
- `src/gui.*`, `src/imgui/` - in-engine debug/editor UI
- `resources/shaders/` - GLSL shader programs
- `resources/models/`, `resources/textures/` - runtime assets
- `Makefile` - build and resource copy workflow

## Known Limitations

- Build settings are debug-oriented (`-O0`) and not performance-optimized.
- Platform support outside macOS is not fully validated yet.

## Roadmap

- Add real-time shadow mapping.
- Expand post-processing effects.
- Improve the asset pipeline for model/texture import workflows.
