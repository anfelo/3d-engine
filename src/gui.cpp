#include "gui.h"
#include "glm/gtc/type_ptr.hpp"

gui GuiCreate(context *Context) {
    // Setup Dear ImGui context
    // -----------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    gui Gui = {.io = ImGui::GetIO()};
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(Context->Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return Gui;
}

void GuiNewFrame(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GuiDraw(context *Context) {
    // Entity Properties
    ImGui::Begin("Entity Properties");
    ImGui::Text("Entity");
    ImGui::DragFloat3("Position", glm::value_ptr(Context->Entity->Position),
                      0.1f);
    ImGui::Separator();
    ImGui::ColorEdit4("Color", glm::value_ptr(Context->Entity->Color));

    ImGui::End();

    // Light Properties
    ImGui::Begin("Light Properties");
    ImGui::Text("Light");
    ImGui::DragFloat3("Position",
                      glm::value_ptr(Context->LightEntity->Position), 0.1f);
    ImGui::Separator();
    ImGui::ColorEdit4("Color", glm::value_ptr(Context->LightEntity->Color));

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiDestroy(void) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
