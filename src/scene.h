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

    skybox Skybox;
};

scene Scene_Create();
void Scene_Destroy(scene &Scene);
void Scene_AddEntity(scene &Scene, entity &Entity);
void Scene_AddLight(scene &Scene, light &Light);

#endif
