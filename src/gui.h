#ifndef GUI_H_
#define GUI_H_

#include "context.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

struct gui {
    ImGuiIO &io;
};

gui Gui_Create(context *Context);
void Gui_NewFrame(void);
void Gui_Draw(context *Context);
void Gui_Destroy(void);

#endif
