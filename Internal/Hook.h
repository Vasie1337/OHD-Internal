#pragma once

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class c_hook {
	typedef long(__stdcall* tPresent)(IDXGISwapChain*, UINT, UINT);
	static inline tPresent originalPresent;

	static inline IDXGISwapChain* pSwapChain = nullptr;
	static inline ID3D11Device* pDevice = nullptr;
	static inline ID3D11DeviceContext* pContext = nullptr;
	static inline ID3D11RenderTargetView* pRenderTargetView = nullptr;
	static inline ID3D11Texture2D* pBackBuffer = nullptr;

	static inline HWND window = nullptr;

	static inline DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};

	static inline bool CheatInit = false;

	static inline WNDPROC oWndProc;
	static inline ImGuiIO io{};

	static LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
	}
	static long __stdcall hookPresent(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags) {

		try {
			if (!CheatInit) {
				SDK::InitGObjects();

				if (!SUCCEEDED(p_swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
					return originalPresent(p_swap_chain, sync_interval, flags);

				pDevice->GetImmediateContext(&pContext);

				if (!SUCCEEDED(p_swap_chain->GetDesc(&SwapChainDesc)))
					return originalPresent(p_swap_chain, sync_interval, flags);

				if (!SUCCEEDED(p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
					return originalPresent(p_swap_chain, sync_interval, flags);

				if (!SUCCEEDED(pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView)))
					return originalPresent(p_swap_chain, sync_interval, flags);

				pBackBuffer->Release();

				window = SwapChainDesc.OutputWindow;
				oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

				ImGui::CreateContext();
				ImGui_ImplWin32_Init(window);
				ImGui_ImplDX11_Init(pDevice, pContext);

				CheatInit = true;
			}

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame();

			c_menu::Tick();
			c_cheat::Tick();

			ImGui::EndFrame();
			ImGui::Render();

			pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
		catch (std::exception& e)
		{
			return originalPresent(p_swap_chain, sync_interval, flags);
		}

		return originalPresent(p_swap_chain, sync_interval, flags);
	}

public:
	static void UnHookPresent()
	{
		if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
			std::cout << "MH_DisableHook error" << std::endl;
			return;
		}
		if (MH_Uninitialize() != MH_OK) {
			std::cout << "MH_Uninitialize error" << std::endl;
		}

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		if (pRenderTargetView) {
			pRenderTargetView->Release();
			pRenderTargetView = nullptr;
		}
		if (pContext) {
			pContext->Release();
			pContext = nullptr;
		}
		if (pDevice) {
			pDevice->Release();
			pDevice = nullptr;
		}

		SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)(oWndProc));
	}
	static bool HookPresent()
	{
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = GetForegroundWindow();
		sd.SampleDesc.Count = 1;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
		if (D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			feature_levels,
			2,
			D3D11_SDK_VERSION,
			&sd,
			&pSwapChain,
			&pDevice,
			nullptr,
			nullptr
		) != S_OK)
		{
			std::cout << "D3D11CreateDeviceAndSwapChain" << std::endl;
			return false;
		}

		void** p_vtable = *reinterpret_cast<void***>(pSwapChain);
		pSwapChain->Release();
		pDevice->Release();

		tPresent targetPresent = reinterpret_cast<tPresent>(p_vtable[8]);

		if (MH_Initialize() != MH_OK) {
			std::cout << "MH_Initialize error" << std::endl;
			return false;
		}

		if (MH_CreateHook(reinterpret_cast<void**>(targetPresent), &hookPresent, reinterpret_cast<void**>(&originalPresent)) != MH_OK) {
			std::cout << "MH_CreateHook error" << std::endl;
			return false;
		}

		if (MH_EnableHook(targetPresent) != MH_OK) {
			std::cout << "MH_EnableHook error" << std::endl;
			return false;
		}

		return true;
	}
};