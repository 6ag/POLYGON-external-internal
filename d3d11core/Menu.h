#pragma once
#include "Singleton.h"
#include "GlobalVars.h"

class Menu : public Singleton <Menu>
{
public:
	void init();
	void switchState();
	void drawMainMenu();
	void endMenuScene();
	WNDPROC wndproc_orig;
	bool showMenu = false;

	// imgui窗口字体
	ImFont * pMenuFont;
	// 透视文字字体
	ImFont * pEspFont;
	// dx刷新颜色
	ImVec4 clearColor = ImGui::ColorConvertU32ToFloat4(ImColor(0, 0, 0, 0));

private:
	bool isInit = false;
};
