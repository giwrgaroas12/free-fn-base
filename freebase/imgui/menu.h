include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "game/settings.hpp"
#include "windows.h"

void menu()
{
	if (GetAsyncKeyState(VK_RSHIFT) & 1)
		options::show_menu = !options::show_menu;

	if (options::show_menu)
	{
		ImGui::SetNextWindowSize(ImVec2(300, 400));
		ImGui::Begin("Free base", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

		ImGui::Checkbox("Corner Box", &options::visuals::cornerbox);
		ImGui::End();
	}
}