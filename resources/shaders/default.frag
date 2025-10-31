#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 u_light_color;
uniform vec3 u_entity_color;
uniform vec3 u_light_pos;

void main()
{
    // ambient
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * u_light_color;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(u_light_pos - FragPos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * u_light_color;

    vec3 result = (ambient + diffuse) * u_entity_color;
    FragColor = vec4(result, 1.0);
}
