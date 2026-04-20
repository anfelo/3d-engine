#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float shininess;
    bool has_specular;
    bool has_normal;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool blinn;
    bool enabled;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool blinn;
    bool enabled;
};
#define NR_POINT_LIGHTS 4

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cut_off;
    float outer_cut_off;

    bool blinn;
    bool enabled;
};

uniform vec3 u_entity_color;
uniform vec3 u_view_pos;
uniform Material u_material;
uniform DirLight u_dir_light;
uniform PointLight u_point_lights[NR_POINT_LIGHTS];
uniform SpotLight u_spot_light;
uniform sampler2D u_shadow_map;

float CalcShadow(vec4 frag_pos_light_space, float bias) {

    // perform perspective divide
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    // transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;

    if (proj_coords.z > 1.0) {
        return 0.0;
    }

    if (proj_coords.x < 0.0 || proj_coords.x > 1.0 || proj_coords.y < 0.0 || proj_coords.y > 1.0) {
        return 0.0;
    }

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closest_depth = texture(u_shadow_map, proj_coords.xy).r;
    // get depth of current fragment from light's perspective
    float current_depth = proj_coords.z;
    // check whether current frag pos is in shadow
    // float shadow = current_depth - bias > closest_depth  ? 1.0 : 0.0;

    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcf_depth = texture(u_shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
            shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

float CalcSpec(vec3 normal, vec3 light_dir, vec3 view_dir, bool use_blinn) {
    float spec = 0.0;
    if (use_blinn) {
        vec3 halfway_dir = normalize(light_dir + view_dir);
        spec = pow(max(dot(normal, halfway_dir), 0.0), 32.0);
    } else {
        vec3 reflect_dir = reflect(-light_dir, normal);
        spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0);
    }
    return spec;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
    if (!light.enabled) {
        return vec3(0.0);
    }

    vec3 light_dir = normalize(light.position - frag_pos);

    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);

    // specular shading
    float spec = CalcSpec(normal, light_dir, view_dir, light.blinn);

    // attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                         light.quadratic * (distance * distance));

    // combine results
    vec3 color = vec3(texture(u_material.diffuse, TexCoords));
    vec3 ambient  = light.ambient  * color;
    vec3 diffuse  = light.diffuse  * diff * color;
    vec3 specular = vec3(0.0);
    if (u_material.has_specular) {
        specular = light.specular * spec * vec3(texture(u_material.specular, TexCoords));
    } else {
        specular = light.specular * spec;
    }

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 view_dir) {
    if (!light.enabled) {
        return vec3(0.0);
    }
    vec3 light_dir = normalize(-light.direction);

    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);

    // specular shading
    float spec = CalcSpec(normal, light_dir, view_dir, light.blinn);

    // combine results
    vec3 color = vec3(texture(u_material.diffuse, TexCoords));
    vec3 ambient  = light.ambient  * color;
    vec3 diffuse  = light.diffuse  * diff * color;
    vec3 specular = vec3(0.0);
    if (u_material.has_specular) {
        specular = light.specular * spec * vec3(texture(u_material.specular, TexCoords));
    } else {
        specular = light.specular * spec;
    }

    // calculate shadow
    float shadow_bias = max(0.005 * (1.0 - dot(normal, light_dir)), 0.0005);
    float shadow = CalcShadow(FragPosLightSpace, shadow_bias);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));

    // return (ambient + diffuse + specular);
    return lighting;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
    if (!light.enabled) {
        return vec3(0.0);
    }

    vec3 light_dir = normalize(light.position - frag_pos);

    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);

    // specular shading
    float spec = CalcSpec(normal, light_dir, view_dir, light.blinn);

    // attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // spot light (soft-edges)
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient = light.ambient * vec3(texture(u_material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(u_material.diffuse, TexCoords));

    vec3 specular = vec3(0.0);
    if (u_material.has_specular) {
        specular = light.specular * spec * vec3(texture(u_material.specular, TexCoords));
    } else {
        specular = light.specular * spec;
    }

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

void main() {
    vec4 tex_color = texture(u_material.diffuse, TexCoords);

    if(tex_color.a < 0.1) {
        discard;
    }

    // properties
    vec3 norm = normalize(Normal);

    if (u_material.has_normal) {
        norm = texture(u_material.normal, TexCoords).rgb;
        // remap from [0,1] to [-1,1]
        norm = normalize(norm * 2.0 - 1.0);
    }

    vec3 view_dir = normalize(u_view_pos - FragPos);

    // phase 1: Directional lighting
    vec3 result = CalcDirLight(u_dir_light, norm, view_dir);

    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += CalcPointLight(u_point_lights[i], norm, FragPos, view_dir);
    }

    // phase 3: Spot light
    result += CalcSpotLight(u_spot_light, norm, FragPos, view_dir);

    // Debug: Shadows
    // float shadow = CalcShadow(FragPosLightSpace);
    // FragColor = vec4(vec3(1.0 - shadow), 1.0);

    FragColor = vec4(result, 1.0);
}
