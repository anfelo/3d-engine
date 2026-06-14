#version 330 core
layout (location = 0) out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec4 ClipSpace;
in vec2 TexCoords;
in vec3 ToCameraVector;

struct Material {
    sampler2D dudv;
    sampler2D normal;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool blinn;
    bool enabled;
    bool casts_shadow;
};

uniform vec3 u_entity_color;
uniform float u_time;
uniform vec3 u_view_pos;
uniform sampler2D u_refraction_texture;
uniform sampler2D u_reflection_texture;
uniform sampler2D u_depth_map;
uniform Material u_material;
uniform DirLight u_dir_light;
uniform float u_near_plane;
uniform float u_far_plane;

const float wave_strength = 0.02;
const float move_speed = 0.03;
const float shine_damper = 20.0;
const float reflectivity = 0.6;

float CalcSpec(vec3 normal, vec3 light_dir, vec3 view_dir, bool use_blinn) {
    float spec = 0.0;
    if (use_blinn) {
        vec3 halfway_dir = normalize(light_dir + view_dir);
        spec = pow(max(dot(normal, halfway_dir), 0.0), shine_damper);
    } else {
        vec3 reflect_dir = reflect(-light_dir, normal);
        spec = pow(max(dot(view_dir, reflect_dir), 0.0), shine_damper);
    }
    return spec;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 view_dir, float water_depth) {
    if (!light.enabled) {
        return vec3(0.0);
    }
    vec3 light_dir = normalize(-light.direction);
    // specular shading
    float spec = CalcSpec(normal, light_dir, view_dir, light.blinn);
    vec3 specular = light.specular * spec * reflectivity * clamp(water_depth / 5.0, 0.0, 1.0);

    return specular;
}

void main() {
    // INFO: not used for now because we have the normal map
    vec3 normal = normalize(Normal);

    vec2 ndc = (ClipSpace.xy / ClipSpace.w) / 2.0 + 0.5;
    vec2 refraction_tex_coords = vec2(ndc.x, ndc.y);
    // Reflection framebuffer was rendered from a mirrored camera, so sample it
    // with the projected Y coordinate flipped inside the [0, 1] texture range.
    vec2 reflection_tex_coords = vec2(ndc.x, 1.0 - ndc.y);

    float depth = texture(u_depth_map, refraction_tex_coords).r;
    float floor_distance = 2.0 * u_near_plane * u_far_plane / (u_far_plane + u_near_plane - (2.0 * depth - 1.0) * (u_far_plane - u_near_plane));

    depth = gl_FragCoord.z;
    float water_distance = 2.0 * u_near_plane * u_far_plane / (u_far_plane + u_near_plane - (2.0 * depth - 1.0) * (u_far_plane - u_near_plane));
    float water_depth = floor_distance - water_distance;

    float move_factor = u_time * move_speed;
    vec2 distorted_tex_coords = texture(u_material.dudv, vec2(TexCoords.x + move_factor, TexCoords.y)).rg * 0.1;
    distorted_tex_coords = TexCoords + vec2(distorted_tex_coords.x, distorted_tex_coords.y + move_factor);
    vec2 total_distortion = (texture(u_material.dudv, distorted_tex_coords).rg * 2.0 - 1.0) * wave_strength * clamp(water_depth / 20.0, 0.0, 1.0);

    refraction_tex_coords += total_distortion;
    reflection_tex_coords += total_distortion;

    vec4 reflect_color = texture(u_reflection_texture, reflection_tex_coords);
    vec4 refract_color = texture(u_refraction_texture, refraction_tex_coords);

    vec4 normal_map = texture(u_material.normal, distorted_tex_coords);
    normal = vec3(normal_map.r * 2.0 - 1.0, normal_map.b * 3.0, normal_map.g * 2.0 - 1.0);
    normal = normalize(normal);

    vec3 view_vector = normalize(ToCameraVector);
    float refraction_factor = dot(view_vector, normal);
    refraction_factor = pow(refraction_factor, 0.5);

    vec3 view_dir = normalize(u_view_pos - WorldPos);
    vec3 light = CalcDirLight(u_dir_light, normal, view_dir, water_depth);

    FragColor = mix(reflect_color, refract_color, refraction_factor);
    // Blueish tint
    FragColor = mix(FragColor, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(light, 0.0);
    FragColor.a = clamp(water_depth / 10.0, 0.0, 1.0);
}
