#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

struct Material {
    samplerCube diffuse;
    samplerCube specular;
    samplerCube normal;
    float shininess;
    bool has_specular;
    bool has_normal;
};

uniform Material u_material;

void main()
{
    FragColor = texture(u_material.diffuse, TexCoords);
}
