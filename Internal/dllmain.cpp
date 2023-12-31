#include <iostream>
#include <windows.h>
#include <d3d11.h>
#include <vector>
#include <thread>
#include <chrono>

#include "MinHook/MinHook.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "../../SDK/SDK.hpp"

#include <D3D11.h>
#include <D3DX11.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DX11.lib")

#include "Menu.h"
#include "Cheat.h"
#include "Hook.h"

void MainThread(HMODULE hModule)
{
	if (!c_hook::HookPresent())
		return;

	while (true) {
		if (GetAsyncKeyState(VK_END) & 0x1)
			break;
		
		if (GetAsyncKeyState(VK_INSERT) & 0x1)
			c_menu::showMenu = !c_menu::showMenu;

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	c_hook::UnHookPresent();

	std::this_thread::sleep_for(std::chrono::seconds(1));

	FreeLibraryAndExitThread(hModule, 0);
}

LONG UEF(PEXCEPTION_POINTERS pExceptionInfo)
{
	return EXCEPTION_CONTINUE_EXECUTION;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)UEF);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
        break;
    }
    return TRUE;
}

