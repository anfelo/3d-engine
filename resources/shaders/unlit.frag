#version 330 core
out vec4 FragColor;

uniform vec3 u_entity_color;

void main()
{
    FragColor = vec4(u_entity_color, 1.0); // set all 4 vector values to 1.0
}
