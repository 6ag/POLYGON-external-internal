#include "Menu.h"
#include "GlobalVars.h"
#include "baidu_font.h"
#include "Renderer.h"

//************************************
// Method:      getDpi
// Description: 获取系统dpi（缩放比例）
// Author:      xxx
// Date:        2020/09/23
// Returns:     比例 
// 内部-1920*1080返回了1.5
//************************************
double getDpi()
{
	double dDpi = 1;
	// Get desktop dc
	HDC desktopDc = GetDC(NULL);
	// Get native resolution
	float horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);
	float verticalDPI = GetDeviceCaps(desktopDc, LOGPIXELSY);
	int dpi = (horizontalDPI + verticalDPI) / 2;
	dDpi = 1 + ((dpi - 96) / 24) * 0.25;
	if (dDpi < 1)
	{
		dDpi = 1;
	}
	::ReleaseDC(NULL, desktopDc);
	return dDpi;
}

void Menu::init()
{
	if (!isInit)
	{
		cout << "屏幕dpi=" << getDpi() << endl;
		// imgui结构检查
		IMGUI_CHECKVERSION();
		// 初始化imgui
		ImGui::CreateContext();
		// 获取imgui
		ImGuiIO & io = ImGui::GetIO();

		// 加载内存字体，并转移所有权
		ImFontConfig f_cfg1;
		f_cfg1.FontDataOwnedByAtlas = false;
		// 好像是最先加载的字体作为菜单窗口的字体
		pMenuFont = io.Fonts->AddFontFromMemoryTTF((void *)baidu_font_data, baidu_font_size, 25.0f, &f_cfg1, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

		ImFontConfig f_cfg2;
		f_cfg2.FontDataOwnedByAtlas = false;
		pEspFont = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 17.0f, &f_cfg2, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

		// 鼠标光标不改变
		io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
		// 不生成imgui.ini窗口位置缓存文件
		io.WantSaveIniSettings = false;
		io.IniFilename = NULL;

	#ifdef EXTERNAL_DRAW
		// 外部绘制默认隐藏菜单
		showMenu = false;
		ImGui::StyleColorsClassic();
	#else
		// 自定义样式
		styleCusom();
	#endif

		// 初始化cpu频率和当前开机时间以及事件类型
		ImGui_ImplWin32_Init(GlobalVars::get().hWindow);
		// 保存dx11到imgui设备和上下文
		ImGui_ImplDX11_Init(Renderer::get().pD3DDevice, Renderer::get().pD3DDeviceContext);

		isInit = true;
		// 测试，默认开启
		if (true)
		{
			boxEsp = true;
			lineEsp = true;
			boneEsp = true;
			distanceEsp = true;
			openFriendEsp = true;
			aimbot = true;
			noRecoil = true;
		}
	}
}

void Menu::imGuiStart()
{
	if (!isInit)
	{
		return;
	}

	// 状态切换，鼠标按下，判断条件也会通过
	switchState();

	// 创建dx11着色器以及字体
	ImGui_ImplDX11_NewFrame();
	// 获取当前系统鼠标 键盘 游戏手柄的信息
	ImGui_ImplWin32_NewFrame();
	// 全局数据更新
	ImGui::NewFrame();

	// 菜单隐藏，不绘制菜单
	if (!showMenu)
	{
		return;
	}
	{
		ImGui::Begin(u8"我爱中国", &showMenu, ImGuiCond_Always | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		ImGui::TextColored(ImColor(220, 20, 60, 255), u8"【Ins】隐藏/显示菜单");
		ImGui::TextColored(ImColor(0, 191, 255, 255), u8"透视选项");
		if (ImGui::BeginTable("split", 2))
		{
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"方框透视  【F1】", &boxEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"连线透视  【F2】", &lineEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"骨头透视  【F3】", &boneEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"距离透视  【F4】", &distanceEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"血条透视", &hpEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"友方透视", &openFriendEsp);
			ImGui::EndTable();
		}

		ImGui::TextColored(ImColor(0, 191, 255, 255), u8"敌人透视颜色");
		static int espColorIndex = 0;
		ImGui::RadioButton(u8"红", &espColorIndex, 0); ImGui::SameLine();
		ImGui::RadioButton(u8"橘", &espColorIndex, 1); ImGui::SameLine();
		ImGui::RadioButton(u8"黄", &espColorIndex, 2); ImGui::SameLine();
		ImGui::RadioButton(u8"蓝", &espColorIndex, 3);
		switch (espColorIndex)
		{
			case 0:
				espColor = Color::Red;
				break;
			case 1:
				espColor = Color::Orange;
				break;
			case 2:
				espColor = Color::Yellow;
				break;
			case 3:
				espColor = Color::Blue;
				break;
			default:
				break;
		}

		ImGui::SliderInt(u8"透视范围", &espRange, 0, 500);

		ImGui::TextColored(ImColor(0, 191, 255, 255), u8"射击选项");
		ImGui::Checkbox(u8"自苗锁定  【F5】", &aimbot);
		ImGui::SameLine();
		ImGui::RadioButton(u8"头部", &aimbotType, 0); ImGui::SameLine();
		ImGui::RadioButton(u8"胸部", &aimbotType, 1);

		ImGui::SliderInt(u8"自苗范围", &aimbotRange, 0, 500);
		ImGui::SliderFloat(u8"射击间隔", &fireSpeed, 0.001f, 1);
		ImGui::Checkbox(u8"无后坐力+快速射击", &noRecoil);
		// 无限子弹好像没伤害
		ImGui::Checkbox(u8"无限子弹", &lockBullet);

		// POLYGON速度改了走不动
		/*ImGui::TextColored(ImColor(0, 191, 255, 255), u8"其他功能");
		ImGui::Text(u8"人物速度");
		ImGui::SameLine();
		if (ImGui::Button(u8"原速"))
		{
			moveSpeed = 1.0f;
			Renderer::get().increaseSpeed();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"2倍速度"))
		{
			moveSpeed = 2.0f;
			Renderer::get().increaseSpeed();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"5倍速度"))
		{
			moveSpeed = 5.0f;
			Renderer::get().increaseSpeed();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"10倍速度"))
		{
			moveSpeed = 10.0f;
			Renderer::get().increaseSpeed();
		}*/
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}

// 热键控制
void Menu::switchState()
{
	//  -32767单点 -32768按下
	SHORT pressed = -32768;
	SHORT click = -32767;

	// 控制窗口显示/隐藏
	if (GetAsyncKeyState(VK_INSERT) == click)
	{
		showMenu = !showMenu;

	#ifdef EXTERNAL_DRAW
		if (showMenu)
		{
			// 显示菜单，让鼠标不穿透透明窗口
			SetWindowLongPtr(GlobalVars::get().overlayHWindow, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
		}
		else
		{
			// 隐藏菜单，让鼠标穿透透明窗口
			SetWindowLongPtr(GlobalVars::get().overlayHWindow, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
		}
	#endif
	}

	// 全开
	if (GetAsyncKeyState(VK_HOME) == click)
	{
		boxEsp = true;
		lineEsp = true;
		boneEsp = true;
		distanceEsp = true;
		openFriendEsp = true;
		aimbot = true;
		noRecoil = true;
		lockBullet = true;
	}

	// 全关
	if (GetAsyncKeyState(VK_END) == click)
	{
		boxEsp = false;
		lineEsp = false;
		boneEsp = false;
		distanceEsp = false;
		openFriendEsp = false;
		aimbot = false;
		noRecoil = false;
		lockBullet = false;
	}

	if (GetAsyncKeyState(VK_F1) == click)
	{
		boxEsp = !boxEsp;
	}

	if (GetAsyncKeyState(VK_F2) == click)
	{
		lineEsp = !lineEsp;
	}

	if (GetAsyncKeyState(VK_F3) == click)
	{
		boneEsp = !boneEsp;
	}

	if (GetAsyncKeyState(VK_F4) == click)
	{
		distanceEsp = !distanceEsp;
	}

	if (GetAsyncKeyState(VK_F5) == click)
	{
		aimbot = !aimbot;
	}
}

void Menu::imGuiEnd()
{
	// 提交imgui绘制数据
	ImGui::Render();
	// 绑定到渲染管线
	Renderer::get().pD3DDeviceContext->OMSetRenderTargets(1, &Renderer::get().pMainRenderTargetView, NULL);
#ifdef EXTERNAL_DRAW
	// 清空屏幕
	Renderer::get().pD3DDeviceContext->ClearRenderTargetView(Renderer::get().pMainRenderTargetView, (float *)&clearColor);
#endif
	// 将imgui的绘制数据绘制到dx11
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#ifdef EXTERNAL_DRAW
	// 呈现，开启垂直同步
	Renderer::get().pSwapChain->Present(1, 0);
#endif
}

// 自定义ImGui样式
void Menu::styleCusom()
{
	auto style = &ImGui::GetStyle();
	// 设置imgui窗口样式
	style->WindowRounding = 5.3f;
	style->GrabRounding = style->FrameRounding = 2.3f;
	style->ScrollbarRounding = 5.0f;
	style->FrameBorderSize = 1.0f;
	style->ItemSpacing.y = 6.5f;
	style->Colors[ImGuiCol_Text] = { 0.73333335f, 0.73333335f, 0.73333335f, 1.00f };
	style->Colors[ImGuiCol_TextDisabled] = { 0.34509805f, 0.34509805f, 0.34509805f, 1.00f };
	style->Colors[ImGuiCol_WindowBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
	style->Colors[ImGuiCol_ChildBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.00f };
	style->Colors[ImGuiCol_PopupBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
	style->Colors[ImGuiCol_Border] = { 0.33333334f, 0.33333334f, 0.33333334f, 0.50f };
	style->Colors[ImGuiCol_BorderShadow] = { 0.15686275f, 0.15686275f, 0.15686275f, 0.00f };
	style->Colors[ImGuiCol_FrameBg] = { 0.16862746f, 0.16862746f, 0.16862746f, 0.54f };
	style->Colors[ImGuiCol_FrameBgHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
	style->Colors[ImGuiCol_FrameBgActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
	style->Colors[ImGuiCol_TitleBg] = { 0.04f, 0.04f, 0.04f, 1.00f };
	style->Colors[ImGuiCol_TitleBgCollapsed] = { 0.16f, 0.29f, 0.48f, 1.00f };
	style->Colors[ImGuiCol_TitleBgActive] = { 0.00f, 0.00f, 0.00f, 0.51f };
	style->Colors[ImGuiCol_MenuBarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.80f };
	style->Colors[ImGuiCol_ScrollbarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.60f };
	style->Colors[ImGuiCol_ScrollbarGrab] = { 0.21960786f, 0.30980393f, 0.41960788f, 0.51f };
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
	style->Colors[ImGuiCol_ScrollbarGrabActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 0.91f };
	style->Colors[ImGuiCol_CheckMark] = { 0.90f, 0.90f, 0.90f, 0.83f };
	style->Colors[ImGuiCol_SliderGrab] = { 0.70f, 0.70f, 0.70f, 0.62f };
	style->Colors[ImGuiCol_SliderGrabActive] = { 0.30f, 0.30f, 0.30f, 0.84f };
	style->Colors[ImGuiCol_Button] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.49f };
	style->Colors[ImGuiCol_ButtonHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
	style->Colors[ImGuiCol_ButtonActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 1.00f };
	style->Colors[ImGuiCol_Header] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.53f };
	style->Colors[ImGuiCol_HeaderHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
	style->Colors[ImGuiCol_HeaderActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
	style->Colors[ImGuiCol_Separator] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	style->Colors[ImGuiCol_SeparatorHovered] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	style->Colors[ImGuiCol_SeparatorActive] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	style->Colors[ImGuiCol_ResizeGrip] = { 1.00f, 1.00f, 1.00f, 0.85f };
	style->Colors[ImGuiCol_ResizeGripHovered] = { 1.00f, 1.00f, 1.00f, 0.60f };
	style->Colors[ImGuiCol_ResizeGripActive] = { 1.00f, 1.00f, 1.00f, 0.90f };
	style->Colors[ImGuiCol_PlotLines] = { 0.61f, 0.61f, 0.61f, 1.00f };
	style->Colors[ImGuiCol_PlotLinesHovered] = { 1.00f, 0.43f, 0.35f, 1.00f };
	style->Colors[ImGuiCol_PlotHistogram] = { 0.90f, 0.70f, 0.00f, 1.00f };
	style->Colors[ImGuiCol_PlotHistogramHovered] = { 1.00f, 0.60f, 0.00f, 1.00f };
	style->Colors[ImGuiCol_TextSelectedBg] = { 0.18431373f, 0.39607847f, 0.79215693f, 0.90f };
}
