#include "stdafx.h"
#include <iostream>

#include "sdl\include\SDL.h"
#include "sdl\include\SDL_syswm.h"
#include "glew\include\glew.h"

#include "imgui\imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdl_gl3.h"

#include "imgui_service.h"

int glMajorVersion = 0;

IMGUI_API bool ImGui_Service_Init(SDL_Window* window) {
	// detect opengl major version
	char* versionStr = (char*)malloc(10);
	memcpy(versionStr, (char*)glGetString(GL_VERSION), strlen((char*)glGetString(GL_VERSION)));
	char* piece = NULL;
	char* nextPiece = NULL;
	piece = strtok_s(versionStr, ".", &nextPiece);
	if (piece != NULL)
		glMajorVersion = atoi(piece);

	if (glMajorVersion < 3)
		return ImGui_ImplSdl_Init(window);
	else
		return ImGui_ImplSdlGL3_Init(window);
}

IMGUI_API void ImGui_Service_Shutdown() {
	if (glMajorVersion < 3)
		return ImGui_ImplSdl_Shutdown();
	else
		return ImGui_ImplSdlGL3_Shutdown();
}

IMGUI_API void ImGui_Service_NewFrame(SDL_Window* window) {
	if (glMajorVersion < 3)
		return ImGui_ImplSdl_NewFrame(window);
	else
		return ImGui_ImplSdlGL3_NewFrame(window);
}

IMGUI_API bool ImGui_Service_ProcessEvent(SDL_Event* event) {
	if (glMajorVersion < 3)
		return ImGui_ImplSdl_ProcessEvent(event);
	else
		return ImGui_ImplSdlGL3_ProcessEvent(event);
}

IMGUI_API void ImGui_Service_InvalidateDeviceObjects() {
	if (glMajorVersion < 3)
		return ImGui_ImplSdl_InvalidateDeviceObjects();
	else
		return ImGui_ImplSdlGL3_InvalidateDeviceObjects();
}

IMGUI_API bool ImGui_Service_CreateDeviceObjects() {
	if (glMajorVersion < 3)
		return ImGui_ImplSdl_CreateDeviceObjects();
	else
		return ImGui_ImplSdlGL3_CreateDeviceObjects();
}