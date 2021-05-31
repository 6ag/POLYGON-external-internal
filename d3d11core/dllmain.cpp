#include "GlobalVars.h"
#include "Renderer.h"
#include "Memory.h"
#include "Menu.h"

using namespace std;

VOID startDebugWindow();
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC oWndProc;
Present oPresent;

// 窗口事件处理
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Menu::get().showMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

// 切换最大化和小窗口会崩溃
// Fatal error: [File:H:/UnrealEngine/Engine/Source/Runtime/Windows/D3D11RHI/Private/D3D11Util.cpp] [Line: 392] SwapChain->ResizeBuffers(0, SizeX, SizeY, RenderTargetFormat, SwapChainFlags) failed at H:/UnrealEngine/Engine/Source/Runtime/Windows/D3D11RHI/Private/D3D11Viewport.cpp:295 with error DXGI_ERROR_INVALID_CALL, (Size=3840x2066 Fullscreen=0 Format=DXGI_FORMAT_R10G10B10A2_UNORM(0x00000018)) -> (Size=2560x1440 Fullscreen=0 Format=DXGI_FORMAT_R10G10B10A2_UNORM(0x00000018))
bool isInitD3DAndImGui = false;
HRESULT __stdcall hkPresent(IDXGISwapChain * pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!isInitD3DAndImGui)
	{
		// 初始化D3D和ImGui
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&Renderer::get().pD3DDevice)))
		{
			Renderer::get().pD3DDevice->GetImmediateContext(&Renderer::get().pD3DDeviceContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			GlobalVars::get().hWindow = sd.OutputWindow;
			ID3D11Texture2D * pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);
			Renderer::get().pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &Renderer::get().pMainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(GlobalVars::get().hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);

			// 初始化imgui
			Menu::get().init();

			isInitD3DAndImGui = true;
		}
		else
		{
			cout << "D3D设备初始化失败" << endl;
			return oPresent(pSwapChain, SyncInterval, Flags);
		}
	}

	// 更新世界地址和矩阵地址
	GlobalVars::get().updateWorldAddrAndViewMatrixAddr();
	// 更新绘制窗口尺寸，不用一直更新。修改窗口尺寸会闪退
	//GlobalVars::get().updateDrawRect();
	// 更新玩家信息
	GlobalVars::get().updatePlayerList();
	// 绘制开始
	Renderer::get().drawFrames();

	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI UpdateThread(HMODULE hModule)
{
	cout << "开始读取数据" << endl;

	// 获取游戏窗口句柄
	GlobalVars::get().hWindow = FindWindow(GAME_WIN_CLASS, GAME_WIN_NAME);

	// 获取进程ID和模块基址
	GlobalVars::get().pId = Memory::get().getProcessId(GlobalVars::get().hWindow);
	GlobalVars::get().baseAddr = Memory::get().GetModuleBaseAddr(MODULE_NAME);

	cout << "游戏进程ID = " << GlobalVars::get().pId << endl;
	cout << "游戏窗口句柄 = " << GlobalVars::get().hWindow << endl;
	cout << "游戏模块基址 = " << GlobalVars::get().baseAddr << endl;

	// 更新世界地址和矩阵地址
	GlobalVars::get().updateWorldAddrAndViewMatrixAddr();
	// 更新绘制窗口尺寸
	GlobalVars::get().updateDrawRect();

	cout << "游戏窗口.x = " << GlobalVars::get().drawRect.x << endl;
	cout << "游戏窗口.y = " << GlobalVars::get().drawRect.y << endl;
	cout << "游戏窗口.width = " << GlobalVars::get().drawRect.width << endl;
	cout << "游戏窗口.height = " << GlobalVars::get().drawRect.height << endl;

	return TRUE;
}

DWORD WINAPI MainThread(HANDLE hModule)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void **)&oPresent, hkPresent);
			init_hook = true;
		}
	}
	while (!init_hook);

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)UpdateThread, hModule, NULL, NULL);
	return TRUE;
}

// 控制台
VOID startDebugWindow()
{
	HINSTANCE g_hInstance = 0;
	HANDLE g_hOutput = 0;
	HWND hwnd = NULL;
	HMENU hmenu = NULL;
	CHAR title[] = "不要关闭本窗口！否则主窗口关闭";
	HANDLE hdlWrite = NULL;

	AllocConsole();
	freopen("CONOUT$", "w+t", stdout);
	freopen("CONIN$", "r+t", stdin);
	g_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	// 设置控制台窗口的属性
	SetConsoleTitle(title);
	SetConsoleTextAttribute((HANDLE)g_hOutput, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	while (NULL == hwnd) hwnd = ::FindWindow(NULL, (LPCTSTR)title);
	// 屏蔽掉控制台窗口的关闭按钮，以防窗口被误删除
	hmenu = ::GetSystemMenu(hwnd, FALSE);
	DeleteMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
	hdlWrite = GetStdHandle(STD_OUTPUT_HANDLE);
	// 这里也可以使用STD_ERROR_HANDLE    TCHAR c[] = {"Hello world!"};WriteConsole(hdlWrite, c, sizeof(c), NULL, NULL);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			startDebugWindow();
			DisableThreadLibraryCalls(hModule);
			CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
			break;
		case DLL_PROCESS_DETACH:
			// 清理工作
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			kiero::shutdown();
			break;
	}
	return TRUE;
}
