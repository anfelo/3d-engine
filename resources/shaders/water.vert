#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;

out vec3 WorldPos;
out vec3 Normal;
out vec4 ClipSpace;
out vec2 TexCoords;
out vec3 ToCameraVector;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_view_pos;
uniform float u_time;
uniform vec4 u_clip_plane;

void main() {
    vec3 pos = a_pos;

    vec4 world_pos = u_model * vec4(pos, 1.0);
    WorldPos = world_pos.xyz;
    ClipSpace = u_projection * u_view * world_pos;
    TexCoords = vec2(a_tex_coords);
    ToCameraVector = u_view_pos - world_pos.xyz;
    Normal = a_normal;

    gl_ClipDistance[0] = dot(world_pos, u_clip_plane);

    gl_Position = ClipSpace;
}
