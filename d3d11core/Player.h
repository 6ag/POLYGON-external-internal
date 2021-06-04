#pragma once
#include "Singleton.h"
#include "GlobalVars.h"

enum class PlayerType
{
	none, // 未知类型
	enemy, // 敌人
	team, // 队友
	other, // 除了自己的人
	oneself // 自己
};

class Player
{
public:
	Player(uintptr_t addres);
	~Player();

	// 基址
	uintptr_t base = 0;

	// 骨骼矩阵地址
	uintptr_t componentToWorldAddr = 0;
	// 骨骼数组地址
	uintptr_t boneArrayAddr = 0;

	// 内存中读取的世界坐标，一般除以100，单位就相当于米
	Vector3 location;
	// 距离
	float distance = 0;
	// 透视方框尺寸信息
	Rect box;
	// 血量
	float hp = 0;
	// 是否是最佳自瞄目标
	bool bestAimTarget = false;
	// 类型
	PlayerType type = PlayerType::none;
	// 蓝图类型名字
	std::string bpCName;

	// 重新去内存中读数据更新
	void update();
private:
	// 获取蓝图类型名字
	std::string getBpCName(uintptr_t base);
};
