#include "material.h"

void Material_Create(material &Material) {
    Material.HasNormalMap = false;
    Material.HasSpecularMap = false;
    Material.ShaderMaterial = shader_material::Default;
    Material.Shininess = 10.0f;
    Material.CullFace = true;
    Material.ReverseNormal = false;
    Material.Color = glm::vec4(1.0f);
}
