#version 330 core

in vec2 v_uv;
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
};

uniform Material u_material;

void main() {
    FragColor =  texture(u_material.diffuse, v_uv);
}
