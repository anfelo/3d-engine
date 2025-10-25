#ifndef GUI_H_
#define GUI_H_

#include "context.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

struct gui {
    ImGuiIO &io;
};

gui GuiCreate(context *Context);
void GuiNewFrame(void);
void GuiDraw(context *Context);
void GuiDestroy(void);

#endif
