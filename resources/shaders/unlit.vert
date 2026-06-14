#version 330 core
layout (location = 0) in vec3 a_pos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec4 u_clip_plane;

void main()
{
    vec4 world_pos = u_model * vec4(a_pos, 1.0);
    gl_ClipDistance[0] = dot(world_pos, u_clip_plane);

    gl_Position = u_projection * u_view * world_pos;
}
