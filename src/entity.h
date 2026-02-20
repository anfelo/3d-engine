#ifndef ENTITY_H_
#define ENTITY_H_

#include <glm/glm.hpp>
#include "mesh.h"
#include "model.h"
#include "texture.h"

enum class entity_type {
    Triangle,
    Quad,
    Cube,
    CubeMesh,
    Model,
};

struct entity {
    entity_type Type;
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::vec4 Rotation;
    glm::vec4 Color;
    bool IsSelected;
    material Material;
    mesh Mesh;
    model Model;
};

enum class light_type {
    Directional,
    Point,
    Spot,
};

struct light {
    entity Entity;
    light_type LightType;
    float AmbientStrength;
    float SpecularStrength;
};

#endif
