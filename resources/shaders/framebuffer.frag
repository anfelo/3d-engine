#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// Use HDR buffer
uniform sampler2D u_screen_texture;
uniform int u_effect;
uniform bool u_hdr_enabled;
uniform float u_exposure;

const float offset = 1.0 / 300.0;
vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right
);

float sharpen_kernel[9] = float[](
    -1, -1, -1,
    -1,  9, -1,
    -1, -1, -1
);

float edge_kernel[9] = float[](
    1, 1, 1,
    1, -8, 1,
    1, 1, 1
);

float blur_kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
);

void main()
{
    const float gamma = 2.2;
    vec4 color = texture(u_screen_texture, TexCoords);

    vec3 sample_tex[9];
    for(int i = 0; i < 9; i++)
    {
        sample_tex[i] = vec3(texture(u_screen_texture, TexCoords.st + offsets[i]));
    }

    switch (u_effect) {
    case 0:
        break;
    case 1:
        // Inversion
        color = vec4(vec3(1.0 - texture(u_screen_texture, TexCoords)), 1.0);
        break;
    case 2:
        // Grayscale
        color = texture(u_screen_texture, TexCoords);
        float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
        color = vec4(average, average, average, 1.0);
        break;
    case 3:
        // Kernel Effects
        // Sharpen
        vec3 col3 = vec3(0.0);
        for(int i = 0; i < 9; i++) {
            col3 += sample_tex[i] * sharpen_kernel[i];
        }
        color = vec4(col3, 1.0);
        break;
    case 4:
        // Blur
        vec3 col4 = vec3(0.0);
        for(int i = 0; i < 9; i++) {
            col4 += sample_tex[i] * blur_kernel[i];
        }
        color = vec4(col4, 1.0);
        break;
    case 5:
        // Edges
        vec3 col5 = vec3(0.0);
        for(int i = 0; i < 9; i++) {
            col5 += sample_tex[i] * edge_kernel[i];
        }
        color = vec4(col5, 1.0);
        break;
    }

    vec3 mapped = color.rgb;
    if(u_hdr_enabled) {
        // exposure tone mapping in linear HDR space
        mapped = vec3(1.0) - exp(-mapped * u_exposure);
    }

    // Gamma correction is the final linear -> sRGB conversion. Do this exactly once.
    mapped = pow(max(mapped, vec3(0.0)), vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}
