#pragma once
#include "Singleton.h"
#include "GlobalVars.h"
#include "Color.h"

class Menu : public Singleton <Menu>
{
public:
	void init();
	void imGuiStart();
	void imGuiEnd();

	bool showMenu = true; // 菜单显示
	int aimbotDistance = 250; // 自瞄距离
	int espDistance = 250; // 透视距离
	int aimbotRadius = 100; // 自瞄圈半径，圆心是屏幕中心
	Color espColor = Color::Red; // 透视颜色
	bool openFriendEsp = false; // 队友透视开关
	bool boxEsp = false; // 方框透视
	bool lineEsp = false; // 连线
	bool hpEsp = false; // 血量透视
	bool distanceEsp = false; // 显示距离
	bool boneEsp = false; // 骨骼透视
	bool aimbot = false; // 自瞄
	int aimbotType = 0; // 0：头部 1：胸部
	float moveSpeed = 1.0f; // 移动速度
	float fireSpeed = 0.001f; // 开火速度
	bool noRecoil = false; // 无后座力
	bool lockBullet = false; // 锁子弹

	// imgui窗口字体
	ImFont * pMenuFont = nullptr;
	// 透视文字字体
	ImFont * pEspFont = nullptr;
#ifdef EXTERNAL_DRAW
	// dx刷新颜色
	ImVec4 clearColor = ImGui::ColorConvertU32ToFloat4(ImColor(0, 0, 0, 0));
#endif

private:
	bool isInit = false;

	void switchState();
	void styleCusom();
};
