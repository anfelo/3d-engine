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
    int width, height, nrChannels;
    unsigned char *data = stbi_load(File, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(Tex->Type, 0, Format, width, height, 0, Format, PixelType,
                     data);
        glGenerateMipmap(Tex->Type);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

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
