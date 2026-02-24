#include "texture.h"

void Texture_Create(texture *Tex, const char *File, GLenum TexType, GLenum Slot,
                    GLenum Format, GLenum PixelType) {
    Tex->Type = TexType;

    // tell stb_image.h to flip loaded texture's on the y-axis.
    stbi_set_flip_vertically_on_load(true);

    glGenTextures(1, &Tex->ID);
    glActiveTexture(Slot);
    glBindTexture(Tex->Type, Tex->ID);

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

        // set the texture wrapping/filtering options (on the currently bound
        // texture object)
        glTexParameteri(Tex->Type, GL_TEXTURE_WRAP_S,
                        Format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(Tex->Type, GL_TEXTURE_WRAP_T,
                        Format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(Tex->Type, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(Tex->Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(Data);

    // Unbinds the OpenGL Texture object so that it can't accidentally be
    // modified
    glBindTexture(Tex->Type, 0);
}

void Texture_CreateCubemap(texture *Tex, std::vector<std::string> Faces) {
    Tex->Type = GL_TEXTURE_CUBE_MAP;

    glGenTextures(1, &Tex->ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Tex->ID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < Faces.size(); i++) {
        unsigned char *data =
            stbi_load(Faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width,
                         height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << Faces[i]
                      << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(Tex->Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(Tex->Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(Tex->Type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(Tex->Type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(Tex->Type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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
