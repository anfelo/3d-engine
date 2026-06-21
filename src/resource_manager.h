#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <map>
#include <string>
#include "model.h"
#include "texture.h"

struct resource_manager {
    std::map<std::string, texture> Textures;
    std::map<std::string, model> Models;

    std::map<shader_type, shader> Shaders;
};

void ResourceManager_LoadShaders(resource_manager &ResourceManager);
void ResourceManager_LoadShader(resource_manager &ResourceManager,
                                shader_type Key, const char *VertexFile,
                                const char *FragmentFile,
                                const char *GeometryFile = nullptr);
void ResourceManager_LoadTextures(resource_manager &ResourceManager);
void ResourceManager_LoadModels(resource_manager &ResourceManager);
void ResourceManager_LoadModel(resource_manager &ResourceManager,
                               const char *File, std::string Key);
void ResourceManager_LoadTexture(resource_manager &ResourceManager,
                                 const char *File, GLenum TexType, GLenum Slot,
                                 GLenum Format, GLenum PixelType,
                                 std::string Name, std::string Key);
void ResourceManager_LoadCubemap(resource_manager &ResourceManager,
                                 std::vector<std::string> Faces,
                                 std::string Key);

const shader *ResourceManager_GetShader(const resource_manager &ResourceManager,
                                  shader_type ShaderType);
texture *ResourceManager_GetTexture(resource_manager &ResourceManager,
                                    std::string Name);
model *ResourceManager_GetModel(resource_manager &ResourceManager,
                                std::string Key);

void ResourceManager_ClearResources(resource_manager &ResourceManager);
#endif
