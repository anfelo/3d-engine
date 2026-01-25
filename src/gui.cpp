#include "gui.h"
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

void Gui_Draw(scene &Scene) {
    // Entity Properties
    ImGui::Begin("Entity Properties");
    for (size_t i = 0; i < Scene.Entities.size(); i++) {
        entity &Entity = Scene.Entities.at(i);

        ImGui::PushID(i);
        ImGui::Text("Entity %d", (int)(i + 1));
        ImGui::Checkbox("Selected", &Entity.IsSelected);
        ImGui::DragFloat3("Position", glm::value_ptr(Entity.Position), 0.1f);
        ImGui::Separator();
        ImGui::ColorEdit4("Color", glm::value_ptr(Entity.Color));
        ImGui::PopID();
    }
    ImGui::End();

    // Light Properties
    ImGui::Begin("Light Properties");
    for (size_t i = 0; i < Scene.Lights.size(); i++) {
        light_entity &Light = Scene.Lights.at(i);

        ImGui::PushID(i);
        ImGui::Text("Light %d", (int)(i + 1));
        ImGui::DragFloat3("Position", glm::value_ptr(Light.Position), 0.1f);
        ImGui::ColorEdit4("Color", glm::value_ptr(Light.Color));

        ImGui::Separator();

        ImGui::Text("Ambient");
        ImGui::DragFloat("Ambient Strength", &Light.AmbientStrength, 0.01f,
                         0.0f, 1.0f);

        ImGui::Separator();

        ImGui::Text("Specular");
        ImGui::DragFloat("Specular Strength", &Light.SpecularStrength, 0.01f,
                         0.0f, 1.0f);
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
