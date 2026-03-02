#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include "texture.h"
#include <glm/glm.hpp>

#define MAX_BONE_INFLUENCE 4

struct vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    // bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct mesh {
    std::vector<vertex> Vertices;
    std::vector<unsigned int> Indices;
    std::vector<texture> Textures;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

void Mesh_Create(mesh *Mesh, std::vector<vertex> Vertices,
                 std::vector<GLuint> Indices, std::vector<texture> Textures);
void Mesh_CreateCube(mesh *Mesh, std::vector<texture> Textures);
void Mesh_CreateQuad(mesh *Mesh, std::vector<texture> Textures);
void Mesh_Setup(mesh *Mesh);
void Mesh_Draw(GLuint ShaderID, mesh *Mesh);
void Mesh_DrawInstance(GLuint ShaderID, mesh *Mesh, unsigned int InstancesNum);

#endif
