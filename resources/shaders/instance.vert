#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;
layout (location = 3) in mat4 a_instance_matrix;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_light_space_matrix;

void main()
{
    FragPos = vec3(a_instance_matrix * vec4(a_pos, 1.0));

    mat3 normal_matrix = mat3(transpose(inverse(a_instance_matrix)));
    Normal = normalize(normal_matrix * a_normal);

    FragPosLightSpace = u_light_space_matrix * vec4(FragPos, 1.0);

    TexCoords = a_tex_coords;

    gl_Position = u_projection * u_view * a_instance_matrix * vec4(a_pos, 1.0);
}
