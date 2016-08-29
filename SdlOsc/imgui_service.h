#ifndef IMGUI_SERVICE_H
#define IMGUI_SERVICE_H

struct SDL_Window;
typedef union SDL_Event SDL_Event;

IMGUI_API bool ImGui_Service_Init(SDL_Window* window);
IMGUI_API void ImGui_Service_Shutdown();
IMGUI_API void ImGui_Service_NewFrame(SDL_Window* window);
IMGUI_API bool ImGui_Service_ProcessEvent(SDL_Event* event);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void ImGui_Service_InvalidateDeviceObjects();
IMGUI_API bool ImGui_Service_CreateDeviceObjects();


#endif
