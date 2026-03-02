#ifndef MODEL_H_
#define MODEL_H_

#include <vector>
#include <assimp/scene.h>
#include "mesh.h"

struct model {
    std::vector<texture> TexturesLoaded;
    std::vector<mesh> Meshes;
    std::string Directory;
    bool GammaCorrection;
};

void Model_Create(model *Model, const char *Path, bool GammaCorrection);
void Model_Load(model *Model, std::string Path);
void Model_Draw(GLuint ShaderID, model *Model);
void Model_DrawInstances(GLuint ShaderID, model *Model,
                         unsigned int InstancesNum);
void Model_ProcessNode(model *Model, aiNode *Node, const aiScene *Scene);
void Model_ProcessMesh(model *Model, mesh *Mesh, aiMesh *Ai_mesh,
                       const aiScene *Scene);
void Model_LoadMaterialTextures(model *Model, std::vector<texture> *Textures,
                                aiMaterial *Mat, aiTextureType Type,
                                std::string TypeName);

#endif
