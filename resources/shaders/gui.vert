#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 2) in vec2 a_uv;

uniform vec2 u_position;
uniform vec2 u_size;
uniform vec2 u_screen_size;

out vec2 v_uv;

void main() {
    vec2 pixel_pos = u_position + a_pos.xy * u_size;

    vec2 ndc;
    ndc.x = (pixel_pos.x / u_screen_size.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (pixel_pos.y / u_screen_size.y) * 2.0;

    gl_Position = vec4(ndc, 0.0, 1.0);

    v_uv = a_uv;
}
