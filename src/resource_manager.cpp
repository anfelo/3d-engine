#include "resource_manager.h"
#include "model.h"
#include "texture.h"
#include <iostream>

void ResourceManager_LoadTextures(resource_manager &ResourceManager) {
    // Crate(Container)
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/container.png", GL_TEXTURE_2D,
        GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE, "diffuse", "container_diffuse");
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/container_specular.png",
        GL_TEXTURE_2D, GL_TEXTURE1, GL_RGBA, GL_UNSIGNED_BYTE, "specular",
        "container_specular");

    // Rock
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/dry_riverbed_rock_diff.png",
        GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE, "diffuse",
        "rock_diffuse");
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/dry_riverbed_rock_normal.png",
        GL_TEXTURE_2D, GL_TEXTURE2, GL_RGBA, GL_UNSIGNED_BYTE, "normal",
        "rock_normal");

    // Grass
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/grass.png", GL_TEXTURE_2D,
        GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE, "diffuse", "grass_diffuse");

    // Window
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/blending_transparent_window.png",
        GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE, "diffuse",
        "window_diffuse");

    // Wood
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/wood.png", GL_TEXTURE_2D,
        GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE, "diffuse", "wood_diffuse");

    // Water
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/water_dudv.png", GL_TEXTURE_2D,
        GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE, "dudv", "water_dudv");
    ResourceManager_LoadTexture(
        ResourceManager, "./resources/textures/water_normal.png", GL_TEXTURE_2D,
        GL_TEXTURE1, GL_RGBA, GL_UNSIGNED_BYTE, "normal", "water_normal");

    // Skyboxes
    // Default
    std::vector<std::string> DefaultSkyboxFaces{
        "./resources/textures/skybox/right.jpg",
        "./resources/textures/skybox/left.jpg",
        "./resources/textures/skybox/top.jpg",
        "./resources/textures/skybox/bottom.jpg",
        "./resources/textures/skybox/front.jpg",
        "./resources/textures/skybox/back.jpg",
    };
    ResourceManager_LoadCubemap(ResourceManager, DefaultSkyboxFaces,
                                "default_skybox");

    // Night
    std::vector<std::string> NightSkyboxFaces{
        "./resources/textures/night_skybox/right.png",
        "./resources/textures/night_skybox/left.png",
        "./resources/textures/night_skybox/top.png",
        "./resources/textures/night_skybox/bottom.png",
        "./resources/textures/night_skybox/front.png",
        "./resources/textures/night_skybox/back.png",
    };
    ResourceManager_LoadCubemap(ResourceManager, NightSkyboxFaces,
                                "night_skybox");
}

void ResourceManager_LoadModels(resource_manager &ResourceManager) {
    ResourceManager_LoadModel(ResourceManager,
                              "./resources/models/backpack/backpack.obj",
                              "backpack_model");
    ResourceManager_LoadModel(ResourceManager,
                              "./resources/models/rock/rock.obj", "rock_model");
}

void ResourceManager_LoadTexture(resource_manager &ResourceManager,
                                 const char *File, GLenum TexType, GLenum Slot,
                                 GLenum Format, GLenum PixelType,
                                 std::string Name, std::string Key) {
    texture Texture;
    Texture_Create(&Texture, File, TexType, Slot, Format, PixelType);
    Texture.Name = Name;

    ResourceManager.Textures.emplace(Key, Texture);
}

void ResourceManager_LoadCubemap(resource_manager &ResourceManager,
                                 std::vector<std::string> Faces,
                                 std::string Key) {
    texture Texture;
    Texture_CreateCubemap(&Texture, Faces);

    ResourceManager.Textures.emplace(Key, Texture);
}

void ResourceManager_LoadModel(resource_manager &ResourceManager,
                               const char *File, std::string Key) {
    auto [Iter, Inserted] = ResourceManager.Models.emplace(Key, model{});
    Model_Create(&Iter->second, File, false);
}

texture *ResourceManager_GetTexture(resource_manager &ResourceManager,
                                    std::string Key) {
    auto Existing = ResourceManager.Textures.find(Key);
    if (Existing == ResourceManager.Textures.end()) {
        std::cerr << "ERROR::RESOURCE_MANAGER:: Texture resource not found: "
                  << Key << std::endl;
        return nullptr;
    }

    return &Existing->second;
}

model *ResourceManager_GetModel(resource_manager &ResourceManager,
                                std::string Key) {
    auto Existing = ResourceManager.Models.find(Key);
    if (Existing == ResourceManager.Models.end()) {
        std::cerr << "ERROR::RESOURCE_MANAGER:: Model resource not found: "
                  << Key << std::endl;
        return nullptr;
    }

    return &Existing->second;
}

void ResourceManager_ClearResources(resource_manager &ResourceManager) {
    for (auto Iter : ResourceManager.Textures) {
        glDeleteTextures(1, &Iter.second.ID);
    }

    // TODO: Clean up the model textures
}
