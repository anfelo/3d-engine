#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "stb_image.h"
#include <glad/glad.h>
#include <iostream>

struct texture {
    GLuint ID;
    GLenum Type;
};

struct material {
    texture DiffuseMap;
    texture NormalMap;
    texture SpecularMap;
    bool HasNormalMap;
    bool HasSpecularMap;
};

void Texture_Create(texture *Tex, const char *File, GLenum TexType, GLenum Slot,
                    GLenum Format, GLenum PixelType);
void Texture_Bind(texture *Tex, GLenum Slot);
void Texture_Uniform(GLuint ShaderID, const char *Uniform, GLuint Unit);
void Texture_Delete(texture *Tex);

#endif
