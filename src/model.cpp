#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>

#include "model.h"
#include "mesh.h"

void Model_Create(model *Model, const char *Path, bool GammaCorrection) {
    Model->GammaCorrection = GammaCorrection;

    Model_Load(Model, std::string(Path));
}

void Model_Load(model *Model, std::string Path) {
    Assimp::Importer Importer;
    const aiScene *Scene = Importer.ReadFile(
        Path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                  aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !Scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << Importer.GetErrorString()
                  << std::endl;
        return;
    }
    Model->Directory = Path.substr(0, Path.find_last_of('/'));

    Model_ProcessNode(Model, Scene->mRootNode, Scene);
}

void Model_Draw(GLuint ShaderID, model *Model) {
    for (unsigned int i = 0; i < Model->Meshes.size(); i++) {
        Mesh_Draw(ShaderID, &Model->Meshes[i]);
    }
}

void Model_ProcessNode(model *Model, aiNode *Node, const aiScene *Scene) {
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < Node->mNumMeshes; i++) {
        aiMesh *Ai_mesh = Scene->mMeshes[Node->mMeshes[i]];
        mesh Mesh;
        Model_ProcessMesh(Model, &Mesh, Ai_mesh, Scene);
        Model->Meshes.push_back(Mesh);
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < Node->mNumChildren; i++) {
        Model_ProcessNode(Model, Node->mChildren[i], Scene);
    }
}

void Model_ProcessMesh(model *Model, mesh *Mesh, aiMesh *Ai_mesh,
                       const aiScene *Scene) {
    // data to fill
    std::vector<vertex> Vertices;
    std::vector<unsigned int> Indices;
    std::vector<texture> Textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < Ai_mesh->mNumVertices; i++) {
        vertex Vertex;
        glm::vec3 Vec3; // we declare a placeholder vector since assimp uses
                        // its own vector class that doesn't directly convert
                        // to glm's vec3 class so we transfer the data to this
                        // placeholder glm::vec3 first.
        // positions
        Vec3.x = Ai_mesh->mVertices[i].x;
        Vec3.y = Ai_mesh->mVertices[i].y;
        Vec3.z = Ai_mesh->mVertices[i].z;
        Vertex.Position = Vec3;
        // normals
        if (Ai_mesh->HasNormals()) {
            Vec3.x = Ai_mesh->mNormals[i].x;
            Vec3.y = Ai_mesh->mNormals[i].y;
            Vec3.z = Ai_mesh->mNormals[i].z;
            Vertex.Normal = Vec3;
        }
        // texture coordinates
        // does the mesh contain texture coordinates?
        if (Ai_mesh->mTextureCoords[0]) {
            glm::vec2 Vec2;
            // a vertex can contain up to 8 different texture coordinates. We
            // thus make the assumption that we won't use models where a vertex
            // can have multiple texture coordinates so we always take the first
            // set (0).
            Vec2.x = Ai_mesh->mTextureCoords[0][i].x;
            Vec2.y = Ai_mesh->mTextureCoords[0][i].y;
            Vertex.TexCoords = Vec2;
            // // tangent
            Vec3.x = Ai_mesh->mTangents[i].x;
            Vec3.y = Ai_mesh->mTangents[i].y;
            Vec3.z = Ai_mesh->mTangents[i].z;
            Vertex.Tangent = Vec3;
            // bitangent
            Vec3.x = Ai_mesh->mBitangents[i].x;
            Vec3.y = Ai_mesh->mBitangents[i].y;
            Vec3.z = Ai_mesh->mBitangents[i].z;
            Vertex.Bitangent = Vec3;
        } else {
            Vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        Vertices.push_back(Vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle)
    // and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < Ai_mesh->mNumFaces; i++) {
        aiFace Face = Ai_mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices
        for (unsigned int j = 0; j < Face.mNumIndices; j++) {
            Indices.push_back(Face.mIndices[j]);
        }
    }
    // process materials
    aiMaterial *Material = Scene->mMaterials[Ai_mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse
    // texture should be named as 'texture_diffuseN' where N is a sequential
    // number ranging from 1 to MAX_SAMPLER_NUMBER. Same applies to other
    // texture as the following list summarizes: diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    Model_LoadMaterialTextures(Model, &Textures, Material,
                               aiTextureType_DIFFUSE, "diffuse");
    // // 2. specular maps
    Model_LoadMaterialTextures(Model, &Textures, Material,
                               aiTextureType_SPECULAR, "specular");
    // 3. normal maps
    Model_LoadMaterialTextures(Model, &Textures, Material, aiTextureType_HEIGHT,
                               "normal");
    // 4. height maps
    Model_LoadMaterialTextures(Model, &Textures, Material,
                               aiTextureType_AMBIENT, "height");

    // return a mesh object created from the extracted mesh data
    Mesh_Create(Mesh, Vertices, Indices, Textures);
}

void Model_LoadMaterialTextures(model *Model, std::vector<texture> *Textures,
                                aiMaterial *Mat, aiTextureType Type,
                                std::string TypeName) {
    std::vector<texture> TexMap;
    for (unsigned int i = 0; i < Mat->GetTextureCount(Type); i++) {
        aiString Str;
        Mat->GetTexture(Type, i, &Str);
        // check if texture was loaded before and if so, continue to next
        // iteration: skip loading a new texture
        bool Skip = false;
        for (unsigned int j = 0; j < Model->TexturesLoaded.size(); j++) {
            if (std::strcmp(Model->TexturesLoaded[j].Path.data(),
                            Str.C_Str()) == 0) {
                Textures->push_back(Model->TexturesLoaded[j]);
                // a texture with the same filepath has already been loaded,
                // continue to next one. (optimization)
                Skip = true;
                break;
            }
        }
        if (!Skip) {
            texture Texture;
            Texture.Path = Str.C_Str();
            Texture.Name = TypeName;

            std::string Filename = std::string(Str.C_Str());
            Filename = Model->Directory + '/' + Filename;

            Texture_Create(&Texture, Filename.c_str(), GL_TEXTURE_2D,
                           GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

            TexMap.push_back(Texture);
            Model->TexturesLoaded.push_back(Texture);
        }
    }

    Textures->insert(Textures->end(), TexMap.begin(), TexMap.end());
}
