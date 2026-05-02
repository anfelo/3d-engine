#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <glm/glm.hpp>
#include "texture.h"

enum class shader_material { Default, Unlit };

struct material {
    std::vector<texture> Textures;

    bool HasNormalMap;
    bool HasSpecularMap;

    glm::vec4 Color;

    shader_material ShaderMaterial;

    float Shininess;
    bool CullFace;
    bool ReverseNormal;
};

void Material_Create(material &Material);

#endif
