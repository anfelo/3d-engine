#ifndef GUI_H_
#define GUI_H_

#include "context.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "scene.h"

struct gui {
    ImGuiIO &io;
};

gui Gui_Create(const context &Context);
void Gui_NewFrame();
void Gui_Draw(scene &Scene);
void Gui_Destroy();

#endif
