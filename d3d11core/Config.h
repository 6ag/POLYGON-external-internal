#pragma once
#include "Singleton.h"
#include "Color.h"

class Config : public Singleton<Config>
{
public:
	int aimbotRange = 250; // 自瞄范围
	int espRange = 250; // 透视范围
	Color espColor = Color::Red; // 透视颜色

	bool openFriendEsp; // 队友透视开关
	bool boxEsp; // 方框透视
	bool lineEsp; // 连线
	bool hpEsp; // 血量透视
	bool distanceEsp; // 显示距离
	bool boneEsp; // 骨骼透视
	bool aimbot; // 自瞄
	int aimbotType = 0; // 0：头部 1：胸部
	float moveSpeed = 1.0f; // 移动速度
	bool noRecoil; // 无后座力
	bool lockHp; // 锁血
	bool lockBullet; // 锁子弹
};
