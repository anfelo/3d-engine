#ifndef ENTITY_H_
#define ENTITY_H_

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

struct entity {
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::vec3 Rotation;
    glm::vec4 Color;
};

#endif
