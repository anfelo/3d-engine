#include "scene.h"
#include "GLFW/glfw3.h"
#include "camera.h"
#include "entity.h"
#include "material.h"

scene Scene_Create() {
    scene Scene = {};

    Scene.HDREnabled = false;
    Scene.HDRExposure = 1.0f;

    return Scene;
}

void Scene_Destroy(scene &Scene) {
    // TODO: Destroy any allocations
}

void Scene_AddEntity(scene &Scene, entity &Entity) {
    Scene.Entities.push_back(Entity);
}

void Scene_AddInstance(scene &Scene, entity &Entity) {
    Scene.Instances.push_back(Entity);
}

void Scene_AddLight(scene &Scene, light &Light) {
    Scene.Lights.push_back(Light);
}

void Scene_AddPointLight(scene &Scene, glm::vec3 Position, glm::vec4 Color,
                         bool IsEnabled = true) {
    material LightDebugMaterial = {};
    Material_Create(LightDebugMaterial);
    LightDebugMaterial.Color = Color;
    LightDebugMaterial.ShaderMaterial = shader_material::Unlit;

    mesh LightDebugMesh;
    Mesh_CreateCube(&LightDebugMesh, LightDebugMaterial);

    light PointLight = {
        .Entity =
            {
                .Type = entity_type::CubeMesh,
                .Position = Position,
                .Scale = glm::vec3(0.2f),
                .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
                .Mesh = LightDebugMesh,
            },
        .Color = Color,
        .LightType = light_type::Point,
        .AmbientStrength = 0.1f,
        .SpecularStrength = 0.5,
        .IsEnabled = IsEnabled,
        .UseBlinn = false,
        .CastsShadow = false,
        .ShowDebug = false,
    };
    Scene_AddLight(Scene, PointLight);
}

void Scene_AddDirectionalLight(scene &Scene, bool IsEnabled = true) {
    light DirectionalLight = {
        .Entity =
            {
                .Position = glm::vec3(0.0f),
                .Scale = glm::vec3(1.0f),
                .Rotation = glm::vec4(0.0f),
            },
        .Color = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f),
        .LightType = light_type::Directional,
        .AmbientStrength = 0.1f,
        .SpecularStrength = 0.5,
        .IsEnabled = IsEnabled,
        .UseBlinn = false,
        .CastsShadow = false,
        .ShowDebug = false,
    };

    Scene_AddLight(Scene, DirectionalLight);
}

void Scene_AddSpotLight(scene &Scene, glm::vec3 Position,
                        bool IsEnabled = true) {
    light SpotLight = {
        .Entity =
            {
                .Position = Position,
                .Scale = glm::vec3(1.0f),
                .Rotation = glm::vec4(0.0f),
            },
        .Color = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f),
        .LightType = light_type::Spot,
        .AmbientStrength = 0.1f,
        .SpecularStrength = 0.5,
        .IsEnabled = IsEnabled,
        .UseBlinn = false,
        .CastsShadow = false,
        .ShowDebug = false,
    };
    Scene_AddLight(Scene, SpotLight);
}

void Scene_BuildScene1(scene &Scene, camera &Camera) {
    // Skybox
    texture SkyboxCubemap;
    std::vector<std::string> SkyboxFaces{
        "./resources/textures/skybox/right.jpg",
        "./resources/textures/skybox/left.jpg",
        "./resources/textures/skybox/top.jpg",
        "./resources/textures/skybox/bottom.jpg",
        "./resources/textures/skybox/front.jpg",
        "./resources/textures/skybox/back.jpg",
    };
    Texture_CreateCubemap(&SkyboxCubemap, SkyboxFaces);
    std::vector<texture> SkyboxTextures = {SkyboxCubemap};

    material SkyboxMaterial = {};
    Material_Create(SkyboxMaterial);
    SkyboxMaterial.Textures = SkyboxTextures;

    mesh SkyboxMesh;
    Mesh_CreateCube(&SkyboxMesh, SkyboxMaterial);

    skybox Skybox = {.Mesh = SkyboxMesh};
    Scene.Skybox = Skybox;

    // Entities
    texture ContainerDiffuseMap;
    Texture_Create(&ContainerDiffuseMap, "./resources/textures/container.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    ContainerDiffuseMap.Name = "diffuse";
    texture ContainerSpecularMap;
    Texture_Create(&ContainerSpecularMap,
                   "./resources/textures/container_specular.png", GL_TEXTURE_2D,
                   GL_TEXTURE1, GL_RGBA, GL_UNSIGNED_BYTE);
    ContainerSpecularMap.Name = "specular";

    std::vector<texture> ContainerTextures = {ContainerDiffuseMap,
                                              ContainerSpecularMap};

    material ContainerMaterial = {};
    Material_Create(ContainerMaterial);
    ContainerMaterial.Textures = ContainerTextures;
    ContainerMaterial.Color = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f);

    mesh ContainerMesh;
    Mesh_CreateCube(&ContainerMesh, ContainerMaterial);

    entity Container = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(0.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(1.0f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Mesh = ContainerMesh,
    };

    Scene_AddEntity(Scene, Container);

    texture RockDiffuseMap;
    Texture_Create(&RockDiffuseMap,
                   "./resources/textures/dry_riverbed_rock_diff.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    RockDiffuseMap.Name = "diffuse";
    texture RockNormalMap;
    Texture_Create(&RockNormalMap,
                   "./resources/textures/dry_riverbed_rock_normal.png",
                   GL_TEXTURE_2D, GL_TEXTURE2, GL_RGBA, GL_UNSIGNED_BYTE);
    RockNormalMap.Name = "normal";

    std::vector<texture> RockTextures = {RockDiffuseMap, RockNormalMap};

    mesh RockMesh = ContainerMesh;
    RockMesh.Material.Textures = RockTextures;
    RockMesh.Material.Color = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f);

    entity Rock = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(2.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(1.0f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Mesh = RockMesh,
    };

    Scene_AddEntity(Scene, Rock);

    model BackpackModel;
    Model_Create(&BackpackModel, "./resources/models/backpack/backpack.obj",
                 false);

    entity Backpack = {
        .Type = entity_type::Model,
        .Position = glm::vec3(4.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(0.4f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Model = BackpackModel,
    };

    Scene_AddEntity(Scene, Backpack);

    model AsteroidModel;
    Model_Create(&AsteroidModel, "./resources/models/rock/rock.obj", false);

    entity Asteroid = {
        .Type = entity_type::Model,
        .Position = glm::vec3(6.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(0.4f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Model = AsteroidModel,
    };

    Scene_AddEntity(Scene, Asteroid);

    unsigned int AsteroidsNum = 1000;
    glm::mat4 *ModelMatrices;
    ModelMatrices = new glm::mat4[AsteroidsNum];
    srand(glfwGetTime());
    float Radius = 50.0;
    float Offset = 2.5f;

    for (unsigned int i = 0; i < AsteroidsNum; i++) {
        // 1. translation: displace along circle with 'radius' in range
        // [-offset, offset]
        float Angle = (float)i / (float)AsteroidsNum * 360.0f;
        float Displacement =
            (rand() % (int)(2 * Offset * 100)) / 100.0f - Offset;
        float X = sin(Angle) * Radius + Displacement;
        Displacement = (rand() % (int)(2 * Offset * 100)) / 100.0f - Offset;
        float Y =
            Displacement *
            0.4f; // keep height of field smaller compared to width of x and z
        Displacement = (rand() % (int)(2 * Offset * 100)) / 100.0f - Offset;
        float Z = cos(Angle) * Radius + Displacement;

        glm::vec3 Position = glm::vec3(X, Y, Z);

        // 2. scale: scale between 0.05 and 0.25f
        glm::vec3 Scale = glm::vec3((rand() % 20) / 100.0f + 0.05);

        // 3. rotation: add random rotation around a (semi)randomly picked
        // rotation axis vector
        float RotationAngle = (rand() % 360);
        glm::vec4 Rotation =
            glm::vec4(RotationAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. Add asteroids to the scene
        entity Asteroid = {
            .Type = entity_type::Model,
            .Position = Position,
            .Scale = Scale,
            .Rotation = Rotation,
            .IsSelected = false,
            .Model = AsteroidModel,
        };

        glm::mat4 Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Position);
        Model = glm::scale(Model, Scale);

        glm::vec3 RotationVec =
            glm::vec3(Rotation[1], Rotation[2], Rotation[3]);
        Model = glm::rotate(Model, glm::radians(Rotation[0]), RotationVec);

        ModelMatrices[i] = Model;

        Scene_AddInstance(Scene, Asteroid);
    }

    // configure instanced array
    // -------------------------
    unsigned int InstancedBuffer;
    glGenBuffers(1, &InstancedBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, InstancedBuffer);
    glBufferData(GL_ARRAY_BUFFER, AsteroidsNum * sizeof(glm::mat4),
                 &ModelMatrices[0], GL_STATIC_DRAW);

    for (unsigned int i = 0; i < AsteroidModel.Meshes.size(); i++) {
        unsigned int VAO = AsteroidModel.Meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (void *)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (void *)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (void *)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (void *)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    texture GrassTexture;
    Texture_Create(&GrassTexture, "./resources/textures/grass.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

    std::vector<texture> GrassTextures = {GrassTexture};

    std::vector<glm::vec3> Vegetation;
    Vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    Vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    Vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    Vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    Vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    material GrassMaterial = {};
    Material_Create(GrassMaterial);
    GrassMaterial.Textures = GrassTextures;

    mesh GrassMesh;
    Mesh_CreateQuad(&GrassMesh, GrassMaterial);

    for (unsigned int i = 0; i < Vegetation.size(); i++) {
        entity Grass = {
            .Type = entity_type::QuadMesh,
            .Position = Vegetation[i],
            .Scale = glm::vec3(1.0f),
            .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
            .Mesh = GrassMesh,
        };

        Scene_AddEntity(Scene, Grass);
    }

    texture WindowTexture;
    Texture_Create(&WindowTexture,
                   "./resources/textures/blending_transparent_window.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

    std::vector<texture> WindowTextures = {WindowTexture};

    std::vector<glm::vec3> Windows;
    Windows.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    Windows.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    Windows.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    Windows.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    Windows.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    glm::vec3 CameraPosition = Camera.Position;
    sort(Windows.begin(), Windows.end(),
         [&CameraPosition](const glm::vec3 &A, const glm::vec3 &B) {
             return length(CameraPosition - A) >= length(CameraPosition - B);
         });

    material WindowMaterial = {};
    Material_Create(WindowMaterial);
    WindowMaterial.Textures = WindowTextures;

    mesh WindowMesh;
    Mesh_CreateQuad(&WindowMesh, WindowMaterial);

    for (unsigned int i = 0; i < Windows.size(); i++) {
        entity Window = {
            .Type = entity_type::QuadMesh,
            .Position = Windows[i],
            .Scale = glm::vec3(1.0f),
            .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
            .Mesh = WindowMesh,
        };

        Scene_AddEntity(Scene, Window);
    }

    // Lights
    glm::vec3 PointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f),
    };
    size_t PointLightPositionsLength =
        sizeof(PointLightPositions) / sizeof(PointLightPositions[0]);

    for (size_t i = 0; i < PointLightPositionsLength; i++) {
        Scene_AddPointLight(Scene, PointLightPositions[i],
                            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    Scene_AddSpotLight(Scene, Camera.Position);
    Scene_AddDirectionalLight(Scene);
}

void Scene_BuildScene2(scene &Scene, camera &Camera) {
    // Skybox
    texture SkyboxCubemap;
    std::vector<std::string> SkyboxFaces{
        "./resources/textures/night_skybox/right.png",
        "./resources/textures/night_skybox/left.png",
        "./resources/textures/night_skybox/top.png",
        "./resources/textures/night_skybox/bottom.png",
        "./resources/textures/night_skybox/front.png",
        "./resources/textures/night_skybox/back.png",
    };
    Texture_CreateCubemap(&SkyboxCubemap, SkyboxFaces);
    std::vector<texture> SkyboxTextures = {SkyboxCubemap};

    material SkyboxMaterial = {};
    Material_Create(SkyboxMaterial);
    SkyboxMaterial.Textures = SkyboxTextures;

    mesh SkyboxMesh;
    Mesh_CreateCube(&SkyboxMesh, SkyboxMaterial);
    skybox Skybox = {.Mesh = SkyboxMesh};
    Scene.Skybox = Skybox;

    // Floor
    texture FloorTexture;
    Texture_Create(&FloorTexture, "./resources/textures/wood.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    FloorTexture.Name = "diffuse";
    FloorTexture.Repeat = glm::vec2(10.0);
    std::vector<texture> FloorTextures = {FloorTexture};

    material FloorMaterial = {};
    Material_Create(FloorMaterial);
    FloorMaterial.Textures = FloorTextures;
    FloorMaterial.Shininess = 16.0f;
    FloorMaterial.Color = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f);

    mesh FloorMesh;
    Mesh_CreateQuad(&FloorMesh, FloorMaterial);

    entity Floor = {
        .Type = entity_type::QuadMesh,
        .Position = glm::vec3(0.0f, -1.0f, 0.0f),
        .Scale = glm::vec3(5.0f),
        .Rotation = glm::vec4(-90.0f, 1.0f, 0.0f, 0.0f),
        .IsSelected = false,
        .Mesh = FloorMesh,
    };

    Scene_AddEntity(Scene, Floor);

    // Entities
    texture ContainerDiffuseMap;
    Texture_Create(&ContainerDiffuseMap, "./resources/textures/container.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    ContainerDiffuseMap.Name = "diffuse";
    texture ContainerSpecularMap;
    Texture_Create(&ContainerSpecularMap,
                   "./resources/textures/container_specular.png", GL_TEXTURE_2D,
                   GL_TEXTURE1, GL_RGBA, GL_UNSIGNED_BYTE);
    ContainerSpecularMap.Name = "specular";

    std::vector<texture> ContainerTextures = {ContainerDiffuseMap,
                                              ContainerSpecularMap};

    material ContainerMaterial = {};
    Material_Create(ContainerMaterial);
    ContainerMaterial.Textures = ContainerTextures;

    mesh ContainerMesh;
    Mesh_CreateCube(&ContainerMesh, ContainerMaterial);

    entity Container1 = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(-0.3f, 0.0f, 0.4f),
        .Scale = glm::vec3(0.4f),
        .Rotation = glm::vec4(60.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Mesh = ContainerMesh,
    };
    entity Container2 = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(1.0f, -0.8f, 1.0f),
        .Scale = glm::vec3(0.4f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Mesh = ContainerMesh,
    };

    Scene_AddEntity(Scene, Container1);
    Scene_AddEntity(Scene, Container2);

    // Lights
    glm::vec3 PointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f),
    };
    size_t PointLightPositionsLength =
        sizeof(PointLightPositions) / sizeof(PointLightPositions[0]);

    for (size_t i = 0; i < PointLightPositionsLength; i++) {
        Scene_AddPointLight(Scene, PointLightPositions[i],
                            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), false);
    }
    Scene_AddSpotLight(Scene, Camera.Position, false);
    Scene_AddDirectionalLight(Scene);
}

void Scene_BuildScene3(scene &Scene, camera &Camera) {
    texture WoodTexture;
    Texture_Create(&WoodTexture, "./resources/textures/wood.png", GL_TEXTURE_2D,
                   GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

    std::vector<texture> ContainerTextures = {WoodTexture};

    material ContainerMaterial = {};
    Material_Create(ContainerMaterial);
    ContainerMaterial.Textures = ContainerTextures;
    ContainerMaterial.CullFace = false;
    ContainerMaterial.ReverseNormal = true;

    mesh ContainerMesh;
    Mesh_CreateCube(&ContainerMesh, ContainerMaterial);

    entity Container = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(0.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(7.0f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Mesh = ContainerMesh,
    };

    Scene_AddEntity(Scene, Container);

    // Entities
    texture BoxDiffuseMap;
    Texture_Create(&BoxDiffuseMap, "./resources/textures/container.png",
                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    BoxDiffuseMap.Name = "diffuse";
    texture BoxSpecularMap;
    Texture_Create(&BoxSpecularMap,
                   "./resources/textures/container_specular.png", GL_TEXTURE_2D,
                   GL_TEXTURE1, GL_RGBA, GL_UNSIGNED_BYTE);
    BoxSpecularMap.Name = "specular";

    std::vector<texture> BoxTextures = {BoxDiffuseMap, BoxSpecularMap};

    material BoxMaterial = {};
    Material_Create(BoxMaterial);
    BoxMaterial.Textures = BoxTextures;

    mesh BoxMesh;
    Mesh_CreateCube(&BoxMesh, BoxMaterial);

    entity Box1 = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(-2.5f, 2.0f, -2.5f),
        .Scale = glm::vec3(0.4f),
        .Rotation = glm::vec4(60.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Mesh = BoxMesh,
    };
    entity Box2 = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(1.0f, -0.8f, 1.0f),
        .Scale = glm::vec3(0.4f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Mesh = BoxMesh,
    };

    Scene_AddEntity(Scene, Box1);
    Scene_AddEntity(Scene, Box2);

    // Lights
    glm::vec3 PointLightPositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f),
    };
    size_t PointLightPositionsLength =
        sizeof(PointLightPositions) / sizeof(PointLightPositions[0]);

    for (size_t i = 0; i < PointLightPositionsLength; i++) {
        Scene_AddPointLight(Scene, PointLightPositions[i],
                            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), i == 0);
    }
    Scene_AddSpotLight(Scene, Camera.Position, false);
    Scene_AddDirectionalLight(Scene, false);
}

void Scene_BuildScene4(scene &Scene, camera &Camera) {
    texture WoodTexture;
    Texture_Create(&WoodTexture, "./resources/textures/wood.png", GL_TEXTURE_2D,
                   GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

    std::vector<texture> ContainerTextures = {WoodTexture};

    material ContainerMaterial = {};
    Material_Create(ContainerMaterial);
    ContainerMaterial.Textures = ContainerTextures;
    ContainerMaterial.CullFace = false;
    ContainerMaterial.ReverseNormal = true;

    mesh ContainerMesh;
    Mesh_CreateCube(&ContainerMesh, ContainerMaterial);

    entity Container = {
        .Type = entity_type::CubeMesh,
        .Position = glm::vec3(0.0f, 0.0f, 0.0f),
        .Scale = glm::vec3(1.0f, 1.0f, 27.0f),
        .Rotation = glm::vec4(0.0f, 1.0f, 0.3f, 0.5f),
        .IsSelected = false,
        .Mesh = ContainerMesh,
    };

    Scene_AddEntity(Scene, Container);

    // Lights
    glm::vec3 PointLightPositions[] = {
        glm::vec3(0.0f, 0.0f, -12.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f),
    };
    glm::vec4 PointLightColors[] = {
        glm::vec4(200.0f, 200.0f, 200.0f, 1.0f),
        glm::vec4(0.1f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 0.0f, 0.2f, 1.0f),
        glm::vec4(0.0f, 0.1f, 0.0f, 1.0f),
    };

    size_t PointLightPositionsLength =
        sizeof(PointLightPositions) / sizeof(PointLightPositions[0]);

    for (size_t i = 0; i < PointLightPositionsLength; i++) {
        Scene_AddPointLight(Scene, PointLightPositions[i], PointLightColors[i],
                            i == 0);
    }
    Scene_AddSpotLight(Scene, Camera.Position, false);
    Scene_AddDirectionalLight(Scene, false);
}
