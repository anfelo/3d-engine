#include "gui.h"
#include "entity.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "scene.h"

gui Gui_Create(const context &Context) {
    // Setup Dear ImGui context
    // -----------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    gui Gui = {.io = ImGui::GetIO()};
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(Context.Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return Gui;
}

void Gui_NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui_Draw(context &Context) {
    int CurrentSceneIdx = Context.CurrentSceneIdx;
    scene *CurrentScene = Context.Scenes.at(CurrentSceneIdx);

    // Entity Properties
    ImGui::Begin("Entity Properties");
    for (size_t i = 0; i < CurrentScene->Entities.size(); i++) {
        entity &Entity = CurrentScene->Entities.at(i);
        if (Entity.Type != entity_type::CubeMesh &&
            Entity.Type != entity_type::Model) {
            continue;
        }

        ImGui::PushID(i);
        ImGui::Text("Entity %d", (int)(i + 1));
        ImGui::Checkbox("Selected", &Entity.IsSelected);
        ImGui::DragFloat3("Position", glm::value_ptr(Entity.Position), 0.1f);
        ImGui::DragFloat4("Rotation", glm::value_ptr(Entity.Rotation), 0.1f);
        ImGui::Separator();
        ImGui::ColorEdit4("Color", glm::value_ptr(Entity.Color));
        ImGui::PopID();
    }
    ImGui::End();

    // Post-Processing
    const char *Effects[] = {"None",           "Inversion", "Grayscale",
                             "Kernel Effects", "Blur",      "Edges"};
    ImGui::Begin("Post-Processing");
    ImGui::Text("Framebuffer Effects");
    ImGui::Combo("##effects", &CurrentScene->Effect, Effects,
                 IM_ARRAYSIZE(Effects));
    ImGui::End();

    // Scenes
    // TODO: Make this dynamic for all the scenes that get added to the context
    const char *Scenes[] = {"Scene1", "Scene2"};
    ImGui::Begin("Scenes");
    ImGui::Combo("##scenes", &Context.CurrentSceneIdx, Scenes,
                 IM_ARRAYSIZE(Scenes));
    ImGui::End();

    // Light Properties
    ImGui::Begin("Light Properties");
    for (size_t i = 0; i < CurrentScene->Lights.size(); i++) {
        light &Light = CurrentScene->Lights.at(i);

        ImGui::PushID(i);
        ImGui::Text("Light %d", (int)(i + 1));
        ImGui::Checkbox("Enabled", &Light.IsEnabled);
        ImGui::Checkbox("Use Blinn", &Light.UseBlinn);
        ImGui::DragFloat3("Position", glm::value_ptr(Light.Entity.Position),
                          0.1f);
        ImGui::ColorEdit4("Color", glm::value_ptr(Light.Entity.Color));

        ImGui::Separator();

        ImGui::Text("Ambient");
        ImGui::DragFloat("Ambient Strength", &Light.AmbientStrength, 0.01f,
                         0.0f, 1.0f);

        ImGui::Separator();

        ImGui::Text("Specular");
        ImGui::DragFloat("Specular Strength", &Light.SpecularStrength, 0.01f,
                         0.0f, 1.0f);

        ImGui::Separator();
        ImGui::Checkbox("Show Debug", &Light.ShowDebug);
        ImGui::Separator();

        ImGui::PopID();
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui_Destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
