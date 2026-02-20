#include "scene.h"

scene Scene_Create() {
    scene Scene = {};

    return Scene;
}

void Scene_Destroy(scene &Scene) {
    // TODO: Destroy any allocations
}

void Scene_AddEntity(scene &Scene, entity &Entity) {
    Scene.Entities.push_back(Entity);
}

void Scene_AddLight(scene &Scene, light &Light) {
    Scene.Lights.push_back(Light);
}
