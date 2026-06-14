#ifndef SCENE_H_
#define SCENE_H_

#include <vector>
#include "camera.h"
#include "entity.h"

struct skybox {
    mesh Mesh;
};

struct scene {
    std::vector<entity> Entities;
    std::vector<light> Lights;
    std::vector<entity> Instances;
    std::vector<entity> GuiTextures;

    int Effect;

    skybox Skybox;

    bool HDREnabled;
    float HDRExposure;

    bool BloomEnabled;
};

scene Scene_Create();
void Scene_Destroy(scene &Scene);
void Scene_AddEntity(scene &Scene, entity &Entity);
void Scene_AddInstance(scene &Scene, entity &Entity);
void Scene_AddGuiTexture(scene &Scene, entity &Entity);
void Scene_AddLight(scene &Scene, light &Light);
void Scene_AddPointLight(scene &Scene, glm::vec3 Position, glm::vec4 Color,
                         bool IsEnabled, bool ShowDebug);
void Scene_AddDirectionalLight(scene &Scene, bool IsEnabled);
void Scene_AddSpotLight(scene &Scene, bool IsEnabled);
void Scene_AddSpotLight(scene &Scene, glm::vec3 Position, bool IsEnabled);

void Scene_BuildScene1(scene &Scene, camera &Camera);
void Scene_BuildScene2(scene &Scene, camera &Camera);
void Scene_BuildScene3(scene &Scene, camera &Camera);
void Scene_BuildScene4(scene &Scene, camera &Camera);
void Scene_BuildScene5(scene &Scene, camera &Camera);
void Scene_BuildScene6(scene &Scene, camera &Camera);

#endif
