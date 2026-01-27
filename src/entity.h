#ifndef ENTITY_H_
#define ENTITY_H_

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

enum class entity_type {
    Triangle,
    Quad,
    Cube,
};

struct entity {
    entity_type Type;
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::vec4 Rotation;
    glm::vec4 Color;
    bool IsSelected;
};

enum class light_type {
    Directional,
    Point,
    Spot,
};

struct light_entity {
    union {
        struct entity Entity;
        struct {
            entity_type Type;
            glm::vec3 Position;
            glm::vec3 Scale;
            glm::vec4 Rotation;
            glm::vec4 Color;
            bool IsSelected;
        };
    };

    light_type LightType;
    float AmbientStrength;
    float SpecularStrength;
};

#endif
