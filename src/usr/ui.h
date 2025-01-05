#pragma once

#include <windows.h>
#include "imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "shared.h"
#include "cam.h"

#define _WINDOWS_
#include "nwin/window.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API bool    ImGui_ImplWin32_Init(void* hwnd);

namespace NWin {
typedef LRESULT  (CALLBACK* win_proc_ptr)(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK defaultWinProc(HWND hwnd, UINT uMsg, 
                                WPARAM wParam, LPARAM lParam);
}

struct LightData {
    fvec3 pos = fvec3(-43.35, 200.750, -112.0);
    fvec3 col = fvec3(1.0, 1.0, 1.0);
};

extern LightData defaultLight;
extern Cam   defaultCam;
extern float ti;

//As NWengine uses directly windows api, we just need to call imgui callback to handle interactions
inline LRESULT CALLBACK imguiProcCallback (HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam)) {
        return 1;
    }
    return NWin::defaultWinProc(hwnd, umsg, wparam, lparam);
};

inline void Init(ImGui_ImplVulkan_InitInfo* info) {
    //Configuration for imgui
    NWin::Window* win = GlobalData::app->win.ptr;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init((HWND)win->_getHandle());
    ImGui_ImplVulkan_Init(info);

    win->setWndProcCallback((void*)&imguiProcCallback);
}

inline void UIBegin() {
     ImGui_ImplVulkan_NewFrame();
     ImGui_ImplWin32_NewFrame();
     ImGui::NewFrame();
}
#include "scene.h"
#include "terrain.h"

extern float FOV;
extern Scene scene;
extern fvec3 obsPos;

inline void UIRender() {   
    ImGui::Begin("inspector");
    ImGui::DragFloat("FOV", &FOV, 0.05);
    ImGui::DragFloat3("Camera Position", &defaultCam.trans.pos.x, 0.05);
    ImGui::DragFloat3("Camera Rotation", &defaultCam.trans.rot.x, 0.05);
    ImGui::DragFloat3("Light Position",  &defaultLight.pos.x, 0.05);
    ImGui::DragFloat("fallout", &defaultLight.col.y, 1.0);
    ImGui::DragFloat("ti", &ti, 1.0);
    if (ImGui::Button("Clean")) {
        scene.objs.begin()->second.back().get<Terrain>()->clean();
    }
    static fvec2 off;
    static fvec3 pos;
    ImGui::DragFloat2("uvoffset", &off.x);
    ImGui::DragFloat3("pos",  &pos.x);
    ImGui::DragFloat3("observer",  &obsPos.x);
    if (ImGui::Button("Add")) {
        scene.objs.begin()->second.back().get<Terrain>()->addChunck(pos,off);
    }
    ImGui::End();
    ImGui::ShowMetricsWindow();
    ImGui::Render();
}

inline void UIEnd() {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GlobalData::cmdBuff->handle);
}



