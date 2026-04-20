#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec2 u_tex_repeat;
uniform mat4 u_light_space_matrix;

void main()
{
    FragPos = vec3(u_model * vec4(a_pos, 1.0));

    mat3 normal_matrix = mat3(transpose(inverse(u_model)));
    Normal = normalize(normal_matrix * a_normal);

    FragPosLightSpace = u_light_space_matrix * vec4(FragPos, 1.0);

    TexCoords = vec2(a_tex_coords.x * u_tex_repeat.x,
                     a_tex_coords.y * u_tex_repeat.y);

    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0);
}
