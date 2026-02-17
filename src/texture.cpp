#include "texture.h"

void Texture_Create(texture *Tex, const char *File, GLenum TexType, GLenum Slot,
                    GLenum Format, GLenum PixelType) {
    Tex->Type = TexType;

    // tell stb_image.h to flip loaded texture's on the y-axis.
    stbi_set_flip_vertically_on_load(true);

    glGenTextures(1, &Tex->ID);
    glActiveTexture(Slot);
    glBindTexture(Tex->Type, Tex->ID);

    // set the texture wrapping/filtering options (on the currently bound
    // texture object)
    glTexParameteri(Tex->Type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(Tex->Type, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(Tex->Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(Tex->Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and generate the texture
    int Width, Height, NrChannels;
    unsigned char *Data = stbi_load(File, &Width, &Height, &NrChannels, 0);
    if (Data) {
        GLenum InternalFormat;
        switch (NrChannels) {
        case 1:
            Format = GL_RED;
            InternalFormat = GL_R8;
            break;
        case 3:
            Format = GL_RGB;
            InternalFormat = GL_RGB8;
            break;
        case 4:
            Format = GL_RGBA;
            InternalFormat = GL_RGBA8;
            break;
        default:
            Format = GL_RGB;
            InternalFormat = GL_RGB8;
            break;
        }
        glTexImage2D(Tex->Type, 0, InternalFormat, Width, Height, 0, Format,
                     PixelType, Data);
        glGenerateMipmap(Tex->Type);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(Data);

    // Unbinds the OpenGL Texture object so that it can't accidentally be
    // modified
    glBindTexture(Tex->Type, 0);
}

void Texture_Uniform(GLuint ShaderID, const char *Uniform, GLuint Unit) {
    GLuint TexUni = glGetUniformLocation(ShaderID, Uniform);
    glUseProgram(ShaderID);

    glUniform1i(TexUni, Unit);
}

void Texture_Bind(texture *Tex, GLenum Slot) {
    glActiveTexture(Slot);
    glBindTexture(Tex->Type, Tex->ID);
}

void Texture_Unbind(texture *Tex) {
    glBindTexture(Tex->Type, 0);
}

void Texture_Delete(texture *Tex) {
    glDeleteTextures(1, &Tex->ID);
}
