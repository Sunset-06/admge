#include "ui.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"


extern "C" void ui_init(SDL_Window* window, SDL_Renderer* renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
}

extern "C" void ui_render_frame(SDL_Texture* emu_texture, SDL_Texture* shell_texture, SDL_Renderer* renderer) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // no title or bg
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    // size of the frame
    ImGui::SetNextWindowSize(ImVec2(600, 900));

    ImGui::Begin("Shell", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    ImVec2 p = ImGui::GetCursorScreenPos();

    // outer gb decoration
    if(!shell_texture)
        printf("no outer image mate\n");

    ImGui::GetWindowDrawList()->AddImage((ImTextureID)shell_texture, p, ImVec2(p.x + 500, p.y + 800));

    ImVec2 screen_pos_min = ImVec2(p.x + 68, p.y + 82); 
    ImVec2 screen_pos_max = ImVec2(p.x + 68 + 320, p.y + 82 + 288);
    
    ImGui::GetWindowDrawList()->AddImage((ImTextureID)emu_texture, screen_pos_min, screen_pos_max);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

extern "C" void ui_cleanup() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}