#include "Menu.h"
#include "GlobalVars.h"
#include "baidu_font.h"
#include "Config.h"
#include "Renderer.h"

void Menu::init()
{
	if (!isInit)
	{
		// 初始化imgui
		ImGui::CreateContext();
		// 获取imgui
		ImGuiIO & io = ImGui::GetIO();

		auto style = &ImGui::GetStyle();

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

		// 初始化cpu频率和当前开机时间以及事件类型
		ImGui_ImplWin32_Init(GlobalVars::get().hWindow);
		// 保存dx11到imgui设备和上下文
		ImGui_ImplDX11_Init(Renderer::get().pD3DDevice, Renderer::get().pD3DDeviceContext);

		isInit = true;
	}
}

void Menu::drawMainMenu()
{
	// 状态切换
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
		ImGui::Begin(u8"我爱中国");
		ImGui::TextColored(ImColor(220, 20, 60, 255), u8"【Ins】隐藏/显示菜单");

		ImGui::TextColored(ImColor(0, 191, 255, 255), u8"透视选项");
		if (ImGui::BeginTable("split", 2))
		{
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"方框透视  【F1】", &Config::get().boxEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"连线透视  【F2】", &Config::get().lineEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"骨头透视  【F3】", &Config::get().boneEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"血条透视  【F4】", &Config::get().hpEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"距离透视  【F5】", &Config::get().distanceEsp);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"友方透视", &Config::get().openFriendEsp);
			ImGui::EndTable();
		}

		ImGui::TextColored(ImColor(0, 191, 255, 255), u8"敌人透视颜色");
		static int espColorIndex = 0;
		ImGui::RadioButton(u8"红", &espColorIndex, 0); ImGui::SameLine();
		ImGui::RadioButton(u8"黄", &espColorIndex, 1); ImGui::SameLine();
		ImGui::RadioButton(u8"蓝", &espColorIndex, 2); ImGui::SameLine();
		ImGui::RadioButton(u8"绿", &espColorIndex, 3);
		switch (espColorIndex)
		{
			case 0:
				Config::get().espColor = Color::Red;
				break;
			case 1:
				Config::get().espColor = Color::Yellow;
				break;
			case 2:
				Config::get().espColor = Color::Blue;
				break;
			case 3:
				Config::get().espColor = Color::Green;
				break;
			default:
				break;
		}

		ImGui::SliderInt(u8"透视范围", &Config::get().espRange, 0, 500);

		ImGui::TextColored(ImColor(0, 191, 255, 255), u8"射击选项");
		ImGui::Checkbox(u8"自苗锁定  【F6】", &Config::get().aimbot);
		ImGui::SameLine();
		ImGui::RadioButton(u8"头部", &Config::get().aimbotType, 0); ImGui::SameLine();
		ImGui::RadioButton(u8"胸部", &Config::get().aimbotType, 1);

		ImGui::SliderInt(u8"自苗范围", &Config::get().aimbotRange, 0, 500);
		ImGui::Checkbox(u8"无后坐力", &Config::get().noRecoil);
		ImGui::Checkbox(u8"无限子弹", &Config::get().lockBullet);

		ImGui::TextColored(ImColor(0, 191, 255, 255), u8"其他功能");
		ImGui::Checkbox(u8"无限血量", &Config::get().lockHp);

		ImGui::Text(u8"人物速度");
		ImGui::SameLine();
		if (ImGui::Button(u8"原速"))
		{
			Config::get().moveSpeed = 1.0f;
			Renderer::get().increaseSpeed();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"2倍速度"))
		{
			Config::get().moveSpeed = 2.0f;
			Renderer::get().increaseSpeed();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"5倍速度"))
		{
			Config::get().moveSpeed = 5.0f;
			Renderer::get().increaseSpeed();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"10倍速度"))
		{
			Config::get().moveSpeed = 10.0f;
			Renderer::get().increaseSpeed();
		}

		ImGui::TextColored(ImColor(220, 20, 60, 255), u8"请隐藏菜单再进行游戏！！！");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}

// 热键控制 & 0x8000：自瞄 & 1：菜单切换
void Menu::switchState()
{
	// 控制窗口显示/隐藏
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		showMenu = !showMenu;
	}

	if (GetAsyncKeyState(VK_F1) & 1)
	{
		Config::get().boxEsp = !Config::get().boxEsp;
	}

	if (GetAsyncKeyState(VK_F2) & 1)
	{
		Config::get().lineEsp = !Config::get().lineEsp;
	}

	if (GetAsyncKeyState(VK_F3) & 1)
	{
		Config::get().boneEsp = !Config::get().boneEsp;
	}

	if (GetAsyncKeyState(VK_F4) & 1)
	{
		Config::get().hpEsp = !Config::get().hpEsp;
	}

	if (GetAsyncKeyState(VK_F5) & 1)
	{
		Config::get().distanceEsp = !Config::get().distanceEsp;
	}

	if (GetAsyncKeyState(VK_F6) & 1)
	{
		Config::get().aimbot = !Config::get().aimbot;
	}

	// 全开
	if (GetAsyncKeyState(VK_HOME & 1))
	{
		Config::get().openFriendEsp = true;
		Config::get().boxEsp = true;
		Config::get().lineEsp = true;
		//Config::get().hpEsp = true;
		Config::get().distanceEsp = true;
		Config::get().boneEsp = true;

		Config::get().aimbot = true;
	}

	// 全关
	if (GetAsyncKeyState(VK_END & 1))
	{
		Config::get().openFriendEsp = false;
		Config::get().boxEsp = false;
		Config::get().lineEsp = false;
		//Config::get().hpEsp = false;
		Config::get().distanceEsp = false;
		Config::get().boneEsp = false;

		Config::get().aimbot = false;
	}
}

void Menu::endMenuScene()
{
	// 提交imgui绘制数据
	ImGui::Render();
	// 绑定到渲染管线
	Renderer::get().pD3DDeviceContext->OMSetRenderTargets(1, &Renderer::get().pMainRenderTargetView, NULL);
	// 清空屏幕，这行要测试
	//Renderer::get().pD3DDeviceContext->ClearRenderTargetView(Renderer::get().pMainRenderTargetView, (float *)&clearColor);

	// 将imgui的绘制数据绘制到dx11
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// 呈现，开启垂直同步，这行要测试
	//Renderer::get().pSwapChain->Present(1, 0);
}
