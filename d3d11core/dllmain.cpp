#include "GlobalVars.h"
#include "Renderer.h"
#include "Memory.h"
#include "Menu.h"

#ifdef EXTERNAL_DRAW
#include <dwmapi.h>
#pragma comment(lib,"Dwmapi.lib")
#endif

using namespace std;

WNDPROC oWndProc;
Present oPresent;

BOOL startFetchData();
VOID startDebugWindow();
void CreateRenderTarget();
void CleanupRenderTarget();
void CleanupDeviceD3D();
bool CreateDeviceD3D();
void initOverlayWindow();

// 开始获取数据
BOOL startFetchData()
{
	cout << "开始读取数据" << endl;

	// 获取游戏窗口句柄
	GlobalVars::get().hWindow = FindWindow(GAME_WIN_CLASS, GAME_WIN_NAME);
	if (GlobalVars::get().hWindow == NULL)
	{
		return FALSE;
	}

	// 获取进程ID和模块基址
	GlobalVars::get().pId = Memory::get().getProcessId(PROCESS_NAME);
	GlobalVars::get().baseAddr = Memory::get().GetModuleBaseAddr(MODULE_NAME);
#ifdef EXTERNAL_DRAW
	GlobalVars::get().hProcess = Memory::get().getProcessHandle(GlobalVars::get().pId);
	cout << "游戏进程句柄 = " << GlobalVars::get().hProcess << endl;
#endif

	cout << "游戏进程ID = " << GlobalVars::get().pId << endl;
	cout << "游戏模块基址 = " << GlobalVars::get().baseAddr << endl;
	cout << "游戏窗口句柄 = " << GlobalVars::get().hWindow << endl;

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

// 调试控制台
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

	// 屏蔽掉控制台窗口的关闭按钮，以防窗口被误删除，会让游戏也关闭
	while (NULL == hwnd)
	{
		hwnd = ::FindWindow(NULL, (LPCTSTR)title);
	}
	hmenu = ::GetSystemMenu(hwnd, FALSE);
	DeleteMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
	hdlWrite = GetStdHandle(STD_OUTPUT_HANDLE);
	// 这里也可以使用STD_ERROR_HANDLE    TCHAR c[] = {"Hello world!"};WriteConsole(hdlWrite, c, sizeof(c), NULL, NULL);
}

// 创建RenderTarget
void CreateRenderTarget()
{
	ID3D11Texture2D * pBackBuffer;
	Renderer::get().pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	Renderer::get().pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &Renderer::get().pMainRenderTargetView);
	pBackBuffer->Release();
}

// 清理RenderTarget
void CleanupRenderTarget()
{
	if (Renderer::get().pMainRenderTargetView)
	{
		Renderer::get().pMainRenderTargetView->Release();
		Renderer::get().pMainRenderTargetView = nullptr;
	}
}

// 清理D3D
void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (Renderer::get().pSwapChain)
	{
		Renderer::get().pSwapChain->Release();
		Renderer::get().pSwapChain = nullptr;
	}
	if (Renderer::get().pD3DDeviceContext)
	{
		Renderer::get().pD3DDeviceContext->Release();
		Renderer::get().pD3DDeviceContext = nullptr;
	}
	if (Renderer::get().pD3DDevice)
	{
		Renderer::get().pD3DDevice->Release();
		Renderer::get().pD3DDevice = nullptr;
	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 窗口事件处理
LRESULT WINAPI WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Menu::get().showMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

#ifdef EXTERNAL_DRAW
	switch (uMsg)
	{
		case WM_SIZE:
			if (Renderer::get().pD3DDevice != NULL && wParam != SIZE_MINIMIZED)
			{
				CleanupRenderTarget();
				Renderer::get().pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
				CreateRenderTarget();
			}
			return false;
			break;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return false;
			break;

		case WM_DESTROY:
			::PostQuitMessage(0);
			return false;
			break;
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
#else
	// 防止窗口缩放崩溃。窗口尺寸变化了，游戏画面尺寸没变，只是为了防止崩溃
	// Fatal error: [File:H:/UnrealEngine/Engine/Source/Runtime/Windows/D3D11RHI/Private/D3D11Util.cpp] [Line: 392] SwapChain->ResizeBuffers(0, SizeX, SizeY, RenderTargetFormat, SwapChainFlags) failed at H:/UnrealEngine/Engine/Source/Runtime/Windows/D3D11RHI/Private/D3D11Viewport.cpp:295 with error DXGI_ERROR_INVALID_CALL, (Size=3840x2066 Fullscreen=0 Format=DXGI_FORMAT_R10G10B10A2_UNORM(0x00000018)) -> (Size=2560x1440 Fullscreen=0 Format=DXGI_FORMAT_R10G10B10A2_UNORM(0x00000018))
	if (uMsg == WM_SIZE)
	{
		if (Renderer::get().pD3DDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			Renderer::get().pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return false;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
#endif
}

// ------------------------------程序入口------------------------------
#ifdef EXTERNAL_DRAW

// 初始化D3D
bool CreateDeviceD3D()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GlobalVars::get().overlayHWindow;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL,
									  D3D_DRIVER_TYPE_HARDWARE,
									  NULL,
									  createDeviceFlags,
									  featureLevelArray,
									  2,
									  D3D11_SDK_VERSION,
									  &sd,
									  &Renderer::get().pSwapChain,
									  &Renderer::get().pD3DDevice,
									  &featureLevel,
									  &Renderer::get().pD3DDeviceContext) != S_OK)
	{
		return false;
	}

	CreateRenderTarget();
	cout << "创建D3D成功 " << endl;
	return true;
}

// 初始化绘制层的窗口
void initOverlayWindow()
{
	auto hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wc;
	wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = HBRUSH(RGB(0, 0, 0));
	wc.lpszClassName = OVERLAY_WIN_CLASS;
	wc.hInstance = hInstance;
	wc.hIconSm = NULL;

	// 注册窗体类
	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "注册窗口类失败", "错误提示", MB_OK | MB_ICONERROR);
		return;
	}

	// WS_EX_LAYERED 可使用SetLayeredWindowAttributes函数
	// WS_EX_TRANSPARENT 鼠标会穿透
	// WS_EX_TOPMOST 窗口置顶
	// WS_EX_TOOLWINDOW 任务栏不显示
	// 创建外部绘制用的窗口
	GlobalVars::get().overlayHWindow = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		OVERLAY_WIN_CLASS,
		OVERLAY_WIN_NAME,
		WS_POPUP | WS_VISIBLE,
		GlobalVars::get().drawRect.x,
		GlobalVars::get().drawRect.y,
		GlobalVars::get().drawRect.width,
		GlobalVars::get().drawRect.height,
		NULL,
		NULL,
		wc.hInstance,
		NULL
	);

	if (GlobalVars::get().overlayHWindow == NULL)
	{
		MessageBox(NULL, "窗口创建失败", "错误提示", MB_OK | MB_ICONERROR);
		return;
	}
	cout << "创建绘制窗口成功 " << endl;

	// 初始化d3d设备
	if (!CreateDeviceD3D())
	{
		//失败的清理
		CleanupDeviceD3D();
		//销毁窗口类
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		MessageBox(NULL, "D3D创建失败", "错误提示", MB_OK | MB_ICONERROR);
		return;
	}

	// 显示窗口
	ShowWindow(GlobalVars::get().overlayHWindow, SW_SHOW);
	// 更新窗体-发送WM_PAINT消息
	UpdateWindow(GlobalVars::get().overlayHWindow);
	// 关键色过滤，要和清空屏幕的颜色一致
	SetLayeredWindowAttributes(GlobalVars::get().overlayHWindow, RGB(0, 0, 0), 255, LWA_ALPHA);
	// 使窗口边框向客户区扩展，也就是距离客户区的4边的边距。4个值为-1，则扩展到整个客户区
	MARGINS marg = { -1, -1, -1, -1 };
	DwmExtendFrameIntoClientArea(GlobalVars::get().overlayHWindow, &marg);

	// imgui窗口透明，也可以解决D3D绘制锯齿
	DWM_BLURBEHIND bb = { 0 };
	HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
	bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
	bb.hRgnBlur = hRgn;
	bb.fEnable = TRUE;
	DwmEnableBlurBehindWindow(GlobalVars::get().overlayHWindow, &bb);

	// 设置DPI，防止绘制内容被缩放，MFC工程不用设置
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

	// 初始化imgui
	Menu::get().init();

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		// 轮询和处理消息
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		GlobalVars::get().hWindow = FindWindow(GAME_WIN_CLASS, GAME_WIN_NAME);
		if (GlobalVars::get().hWindow == NULL)
		{
			// 游戏窗口关闭，跳出循环，释放资源
			break;
		}

		// 游戏窗口或绘制窗口没在最前端，暂停绘制。还可以方便开发的时候获取绘制的一些地址、数值
		HWND hwnd = GetForegroundWindow();
		if (GlobalVars::get().hWindow == hwnd || GlobalVars::get().overlayHWindow == hwnd)
		{
			// 更新世界地址和矩阵地址
			GlobalVars::get().updateWorldAddrAndViewMatrixAddr();
			// 更新绘制窗口尺寸。内部修改窗口尺寸无效
			GlobalVars::get().updateDrawRect();
			// 更新玩家信息
			GlobalVars::get().updatePlayerList();
			// 绘制开始
			Renderer::get().drawFrames();

			// 使辅助窗口一直盖在游戏窗口上，这段代码需要降低执行频率
			SetWindowPos(GlobalVars::get().overlayHWindow, HWND_TOPMOST, GlobalVars::get().drawRect.x, GlobalVars::get().drawRect.y, GlobalVars::get().drawRect.width, GlobalVars::get().drawRect.height, SWP_SHOWWINDOW);
			MoveWindow(GlobalVars::get().overlayHWindow, GlobalVars::get().drawRect.x, GlobalVars::get().drawRect.y, GlobalVars::get().drawRect.width, GlobalVars::get().drawRect.height, true);
		}
		//Sleep(1);
	}

	// 清理工作
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(GlobalVars::get().overlayHWindow);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}

// 可执行程序入口
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	startDebugWindow();
	if (startFetchData())
	{
		initOverlayWindow();
	}
	return 0;
}

#else

// 切换最大化和小窗口会崩溃
bool isInitD3DAndImGui = false;
HRESULT __stdcall hkPresent(IDXGISwapChain * pSwapChain, UINT SyncInterval, UINT Flags)
{
	Renderer::get().pSwapChain = pSwapChain;
	if (!isInitD3DAndImGui)
	{
		// 初始化D3D和ImGui
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&Renderer::get().pD3DDevice)))
		{
			Renderer::get().pD3DDevice->GetImmediateContext(&Renderer::get().pD3DDeviceContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			GlobalVars::get().hWindow = sd.OutputWindow;

			// 创建RenderTarget
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
	// 更新绘制窗口尺寸。内部修改窗口尺寸无效。游戏内部画面不会变
	//GlobalVars::get().updateDrawRect();
	// 更新玩家信息
	GlobalVars::get().updatePlayerList();
	// 绘制开始
	Renderer::get().drawFrames();

	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI mainThread(HANDLE hModule)
{
	startFetchData();

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
	return TRUE;
}

// DLL程序入口
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			//startDebugWindow();
			DisableThreadLibraryCalls(hModule);
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)mainThread, hModule, NULL, NULL);
			break;
		case DLL_PROCESS_DETACH:
			kiero::unbind(8);
			kiero::shutdown();

			if (oWndProc != nullptr)
			{
				// 劫持的窗口事件还回去，否则会闪退
				SetWindowLongPtr(GlobalVars::get().hWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
				oWndProc = nullptr;
			}

			CleanupDeviceD3D();
			if (oPresent != nullptr)
			{
				oPresent = nullptr;
			}

			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			break;
	}
	return TRUE;
}
#endif
