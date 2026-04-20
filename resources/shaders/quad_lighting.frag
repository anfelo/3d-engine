#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float shininess;
    bool has_specular;
    bool has_normal;
};

uniform Material u_material;

void main()
{
    vec4 tex_color = texture(u_material.diffuse, TexCoords);

    if(tex_color.a < 0.1) {
        discard;
    }

    FragColor = tex_color;
}
