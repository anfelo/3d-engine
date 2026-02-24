#include "mesh.h"

void Mesh_Create(mesh *Mesh, std::vector<vertex> Vertices,
                 std::vector<GLuint> Indices, std::vector<texture> Textures) {
    Mesh->Vertices = Vertices;
    Mesh->Indices = Indices;
    Mesh->Textures = Textures;

    Mesh_Setup(Mesh);
}

void Mesh_Setup(mesh *Mesh) {
    // create buffers/arrays
    glGenVertexArrays(1, &Mesh->VAO);
    glGenBuffers(1, &Mesh->VBO);
    glGenBuffers(1, &Mesh->EBO);

    glBindVertexArray(Mesh->VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    // A great thing about structs is that their memory layout is sequential for
    // all its items. The effect is that we can simply pass a pointer to the
    // struct and it translates perfectly to a glm::vec3/2 array which again
    // translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, Mesh->Vertices.size() * sizeof(vertex),
                 &Mesh->Vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 Mesh->Indices.size() * sizeof(unsigned int), &Mesh->Indices[0],
                 GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void *)offsetof(vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void *)offsetof(vertex, TexCoords));
    // vertex tangent
    // glEnableVertexAttribArray(3);
    // glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
    //                       (void *)offsetof(vertex, Tangent));
    // vertex bitangent
    // glEnableVertexAttribArray(4);
    // glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
    //                       (void *)offsetof(vertex, Bitangent));
    // ids
    // glEnableVertexAttribArray(5);
    // glVertexAttribIPointer(5, 4, GL_INT, sizeof(vertex),
    //                        (void *)offsetof(vertex, M_BoneIDs));

    // weights
    // glEnableVertexAttribArray(6);
    // glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(vertex),
    //                       (void *)offsetof(vertex, M_Weights));
    glBindVertexArray(0);
}

void Mesh_Draw(GLuint ShaderID, mesh *Mesh) {
    // bind appropriate textures
    unsigned int DiffuseNr = 1;
    unsigned int SpecularNr = 1;
    unsigned int NormalNr = 1;
    unsigned int HeightNr = 1;

    glUniform1i(glGetUniformLocation(ShaderID, "u_material.has_specular"), 0);
    glUniform1i(glGetUniformLocation(ShaderID, "u_material.has_normal"), 0);

    for (unsigned int i = 0; i < Mesh->Textures.size(); i++) {
        // active proper texture unit before binding
        glActiveTexture(GL_TEXTURE0 + i);
        // retrieve texture number (the N in diffuse_textureN)
        std::string Number;
        std::string Name = Mesh->Textures[i].Name;
        if (Name == "diffuse") {
            Number = std::to_string(DiffuseNr++);
        } else if (Name == "specular") {
            // transfer unsigned int to string
            Number = std::to_string(SpecularNr++);
            glUniform1i(
                glGetUniformLocation(ShaderID, "u_material.has_specular"), 1);
        } else if (Name == "normal") {
            // transfer unsigned int to string
            Number = std::to_string(NormalNr++);
            glUniform1i(glGetUniformLocation(ShaderID, "u_material.has_normal"),
                        1);
        } else if (Name == "height") {
            // transfer unsigned int to string
            Number = std::to_string(HeightNr++);
        }
        // now set the sampler to the correct texture unit
        glUniform1i(
            glGetUniformLocation(ShaderID, ("u_material." + Name).c_str()), i);
        // and finally bind the texture
        glBindTexture(Mesh->Textures[i].Type, Mesh->Textures[i].ID);
    }

    // draw mesh
    glBindVertexArray(Mesh->VAO);
    glDrawElements(GL_TRIANGLES,
                   static_cast<unsigned int>(Mesh->Indices.size()),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void Mesh_CreateCube(mesh *Mesh, std::vector<texture> Textures) {
    std::vector<vertex> Vertices = {
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
         glm::vec2(0.0f, 1.0f)},
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
         glm::vec2(0.0f, 1.0f)},
    };

    std::vector<GLuint> Indices = {// Front face
                                   0, 2, 1, 2, 5, 4,
                                   // Back face
                                   6, 7, 8, 8, 10, 11,
                                   // Left face
                                   12, 13, 14, 14, 16, 17,
                                   // Right face
                                   18, 20, 19, 20, 23, 22,
                                   // Bottom face
                                   24, 25, 26, 26, 28, 29,
                                   // Top face
                                   30, 32, 31, 32, 35, 34};

    Mesh_Create(Mesh, Vertices, Indices, Textures);
}

void Mesh_CreateQuad(mesh *Mesh, std::vector<texture> Textures) {
    std::vector<vertex> Vertices = {
        {glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec2(0.0f, 1.0f)},
    };

    std::vector<GLuint> Indices = {0, 1, 2, 2, 3, 0};

    Mesh_Create(Mesh, Vertices, Indices, Textures);
}
