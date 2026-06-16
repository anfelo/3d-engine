#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <map>
#include <string>
#include "model.h"
#include "texture.h"

struct resource_manager {
    std::map<std::string, texture> Textures;
    std::map<std::string, model> Models;
};

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

texture *ResourceManager_GetTexture(resource_manager &ResourceManager,
                                    std::string Name);
model *ResourceManager_GetModel(resource_manager &ResourceManager,
                                std::string Key);

void ResourceManager_ClearResources(resource_manager &ResourceManager);
#endif
