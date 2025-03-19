
#include <iostream>
#include "Windows.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "game/sdk.hpp"
#include "driver.hpp"

#include <d3d9.h>
#include "Uxtheme.h"
#include "dwmapi.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")
HWND my_wnd = NULL;
HWND game_wnd = NULL;
MSG messager = { NULL };
IDirect3D9Ex* p_object = NULL;
IDirect3DDevice9Ex* p_device = NULL;
D3DPRESENT_PARAMETERS p_params = { NULL };

HWND get_process_wnd(uint32_t pid)
{
	std::pair<HWND, uint32_t> params = { 0, pid };
	BOOL bresult = EnumWindows([](HWND hwnd, LPARAM lparam) -> BOOL
		{
			auto pparams = (std::pair<HWND, uint32_t>*)(lparam);
			uint32_t processid = 0;
			if (GetWindowThreadProcessId(hwnd, reinterpret_cast<LPDWORD>(&processid)) && processid == pparams->second)
			{
				SetLastError((uint32_t)-1);
				pparams->first = hwnd;
				return FALSE;
			}
			return TRUE;
		}, (LPARAM)&params);
	if (!bresult && GetLastError() == -1 && params.first) return params.first;
	return 0;
}


HRESULT directx_init()
{
	// Initialize Direct3D 9
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_object)))
		exit(3);  // Exit if failed to create Direct3D object
	// Set up parameters for device creation
	ZeroMemory(&p_params, sizeof(p_params));
	p_params.Windowed = TRUE;
	p_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_params.hDeviceWindow = my_wnd;
	p_params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_params.BackBufferWidth = options::width;
	p_params.BackBufferHeight = options::height;
	p_params.EnableAutoDepthStencil = TRUE;
	p_params.AutoDepthStencilFormat = D3DFMT_D16;
	p_params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// Create the Direct3D device
	if (FAILED(p_object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, my_wnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_params, nullptr, &p_device)))
	{
		p_object->Release();  // Release the Direct3D object on failure
		exit(4);
	}

	// Initialize ImGui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	io.Fonts->AddFontDefault();


	ImGui_ImplWin32_Init(my_wnd);
	ImGui_ImplDX9_Init(p_device);

	// Clean up Direct3D object after ImGui initialization
	p_object->Release();

	return S_OK;
}


auto create_overlay() -> bool
{
	WNDCLASSEXA wcex = {
	  sizeof(WNDCLASSEXA),
	  0,
	  DefWindowProcA,
	  0,
	  0,
	  nullptr,
	  LoadIcon(nullptr, IDI_APPLICATION),
	  LoadCursor(nullptr, IDC_ARROW),
	  nullptr,
	  nullptr,
	  ("Discord"),
	  LoadIcon(nullptr, IDI_APPLICATION)
	};

	RECT Rect;
	GetWindowRect(GetDesktopWindow(), &Rect);

	RegisterClassExA(&wcex);

	my_wnd = CreateWindowExA(NULL, ("Discord"), ("Bald Niggas Association - Server"), WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);


	SetWindowLong(my_wnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(my_wnd, &margin);
	ShowWindow(my_wnd, SW_SHOW);
	SetWindowPos(my_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetLayeredWindowAttributes(my_wnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	UpdateWindow(my_wnd);
	return true;
}

void draw_cornered_box(int x, int y, int w, int h, const ImColor color, int thickness)
{
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x, y + (h / 3)), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x + (w / 3), y), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y), ImVec2(x + w, y), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + (h / 3)), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y + h - (h / 3)), ImVec2(x, y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y + h), ImVec2(x + (w / 3), y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y + h), ImVec2(x + w, y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w, y + h - (h / 3)), ImVec2(x + w, y + h), color, thickness);
}

void game_loop()
{
	cache::uworld = read<uintptr_t>(base + UWORLD);

	//printf("UWorld: %p\n", cache::uworld);
	cache::game_instance = read<uintptr_t>(cache::uworld + GAME_INSTANCE);
	cache::local_players = read<uintptr_t>(read<uintptr_t>(cache::game_instance + LOCAL_PLAYERS));
	cache::player_controller = read<uintptr_t>(cache::local_players + PLAYER_CONTROLLER);
	cache::local_pawn = read<uintptr_t>(cache::player_controller + LOCAL_PAWN);

	if (cache::local_pawn != 0)
	{
		cache::root_component = read<uintptr_t>(cache::local_pawn + ROOT_COMPONENT);
		cache::relative_location = read<Vector3>(cache::root_component + RELATIVE_LOCATION);
		cache::player_state = read<uintptr_t>(cache::local_pawn + PLAYER_STATE);
		cache::my_team_id = read<int>(cache::player_state + TEAM_INDEX);
	}

	cache::game_state = read<uintptr_t>(cache::uworld + GAME_STATE);
	cache::player_array = read<uintptr_t>(cache::game_state + PLAYER_ARRAY);
	cache::player_count = read<int>(cache::game_state + (PLAYER_ARRAY + sizeof(uintptr_t)));
	cache::relative_location = read<Vector3>(cache::root_component + RELATIVE_LOCATION);
	cache::current_weapon = read<uintptr_t>(cache::local_pawn + CURRENT_WEAPON);

	cache::closest_distance = FLT_MAX;
	cache::closest_mesh = NULL;
	cache::closest_entity = NULL;

	for (int i = 0; i < cache::player_count; i++)
	{
		uintptr_t player_state = read<uintptr_t>(cache::player_array + (i * sizeof(uintptr_t)));
		if (!player_state)
			continue;

		int player_team_id = read<int>(player_state + TEAM_INDEX);



		uintptr_t pawn_private = read<uintptr_t>(player_state + PAWN_PRIVATE);
		if (!pawn_private || pawn_private == cache::local_pawn)
			continue;

		uintptr_t mesh = read<uintptr_t>(pawn_private + MESH);
		if (!mesh)
			continue;

		Vector3 head3d = get_entity_bone(mesh, 110);
		Vector2 head2d = project_world_to_screen(Vector3(head3d.x, head3d.y, head3d.z + 20));
		Vector3 bottom3d = get_entity_bone(mesh, 0);
		Vector2 bottom2d = project_world_to_screen(bottom3d);
		float box_height = abs(head2d.y - bottom2d.y);
		float box_width = box_height * 0.50f;
		float distance = cache::relative_location.distance(bottom3d) / 100;

		// your options here like corner box and shit

		if (options::visuals::cornerbox)
		{
				draw_cornered_box(head2d.x - (box_width / 2), head2d.y, box_width, box_height, ImColor(250, 0, 0, 250), 1);
		}
	}
}

void menu()
{

	ImGui::SetNextWindowSize(ImVec2(300, 400));
	ImGui::Begin("Free github base", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	ImGui::Checkbox("Corner Box", &options::visuals::cornerbox);
	ImGui::End();
}
WPARAM render_loop()
{
	static RECT old_rc;
	ZeroMemory(&messager, sizeof(MSG));
	while (messager.message != WM_QUIT)
	{
		if (PeekMessage(&messager, my_wnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&messager);
			DispatchMessage(&messager);
		}
		if (game_wnd == NULL) exit(0);
		HWND active_wnd = GetForegroundWindow();
		///if (active_wnd == game_wnd)
		//{
		//	HWND target_wnd = GetWindow(active_wnd, GW_HWNDPREV);
		//	SetWindowPos(my_wnd, target_wnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		//}
		//else
		//{
		//	game_wnd = get_process_wnd(processID);
		//	Sleep(250);
		//}
		RECT rc;
		POINT xy;
		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		rc.left = xy.x;
		rc.top = xy.y;
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = 1.0f / 60.0f;
		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;
		if (GetAsyncKeyState(0x1))
		{
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
		{
			io.MouseDown[0] = false;
		}
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;
			options::width = rc.right;
			options::height = rc.bottom;
			p_params.BackBufferWidth = options::width;
			p_params.BackBufferHeight = options::height;
			SetWindowPos(my_wnd, (HWND)0, xy.x, xy.y, options::width, options::height, SWP_NOREDRAW);
			p_device->Reset(&p_params);
		}
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		game_loop();
		menu();
		ImGui::EndFrame();
		p_device->SetRenderState(D3DRS_ZENABLE, false);
		p_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		p_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		p_device->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		if (p_device->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			p_device->EndScene();
		}
		HRESULT result = p_device->Present(0, 0, 0, 0);
		if (result == D3DERR_DEVICELOST && p_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_device->Reset(&p_params);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (p_device != 0)
	{
		p_device->EndScene();
		p_device->Release();
	}
	if (p_object != 0) p_object->Release();
	DestroyWindow(my_wnd);
	return messager.wParam;
}

int main()
{
	if (!mem::find_driver())
	{
		system("color 4");
		Beep(300, 500);
		printf("load drv dummy");
		system("pause >nul");
	}
	else
	{
		system("color 2");
		Beep(500, 300);
		printf("drv is loaded!\n");
	}

	mem::process_id = mem::FindProcess(L"FortniteClient-Win64-Shipping.exe");
	game_wnd = get_process_wnd(mem::process_id); //found the process

	auto cr3 = mem::fetch_cr3();
	mem::fetch_cr3(); //loading cr3

	base = mem::base();
	
	create_overlay();
	directx_init();
	render_loop();

}


