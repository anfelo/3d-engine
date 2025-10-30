#version 330 core
out vec4 FragColor;

uniform vec3 u_light_color;
uniform vec3 u_entity_color;

void main()
{
    // ambient
    float ambient_strength = 1.0;
    vec3 ambient = ambient_strength * u_light_color;

    vec3 result = ambient * u_entity_color;
    FragColor = vec4(result, 1.0);
}
