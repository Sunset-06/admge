#include "ui.h"
#include "emu.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"


extern "C" void ui_init(SDL_Window* window, SDL_Renderer* renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
}

extern "C" void ui_render(SDL_Texture* emu_texture, SDL_Texture* shell_texture, SDL_Renderer* renderer, CPU *cpu) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // no title or bg
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    // send size of the frame
    ImGui::SetNextWindowSize(ImVec2(1236*win_scale, 1072*win_scale));
    // send padding = 0
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground;

    if (rom_loaded) {
        // This allows clicks to pass through the UI window to the game
        window_flags |= ImGuiWindowFlags_NoInputs;
    }


    ImGui::Begin("Shell", nullptr, window_flags);

    ImVec2 p = ImGui::GetCursorScreenPos();

    // outer gb decoration
    if(!shell_texture)
        printf("no outer image found\n");

    // for the modes
    float shell_w, shell_h;
    float offset_x, offset_y;
    float screen_w, screen_h;
    
    switch(current_mode){
        case MGB: shell_w = 872; shell_h = 800;
                  offset_x = 158; offset_y = 152;
                  screen_w = 557; screen_h= 492;
                  break;

        case DMG:
        default:  shell_w = 900; shell_h =750;
                  offset_x = 205; offset_y = 185;
                  screen_w = 485; screen_h = 420;
    }
    // outer border image
    ImGui::GetWindowDrawList()->AddImage((ImTextureID)shell_texture, p, ImVec2(p.x + shell_w, p.y + shell_h));
    // inner screen texture
    ImVec2 screen_pos_min = ImVec2(p.x + offset_x, p.y + offset_y);
    ImVec2 screen_pos_max = ImVec2(p.x + offset_x + screen_w,  p.y + offset_y + screen_h);
    if(rom_loaded){
        ImGui::GetWindowDrawList()->AddImage((ImTextureID)emu_texture, screen_pos_min, screen_pos_max);
    }
    else {
      ImGui::SetCursorScreenPos(ImVec2(0,0));
      if (ImGui::Button("Load a Cartridge")){
        const char* filters[] = {"*.gb"};
        const char* path = tinyfd_openFileDialog(
            "Select your ROM",
            "",
            1,
            filters,
            "ROM Files",
            0
        );


        if(path){
          load_rom(cpu, path);
          rom_loaded = true;
        }
      }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}


extern "C" void ui_handle_event(SDL_Event* event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

extern "C" bool ui_want_capture() {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard || io.WantCaptureMouse;
}
extern "C" void ui_cleanup() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
