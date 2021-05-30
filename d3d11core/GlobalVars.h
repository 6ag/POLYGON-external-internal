#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <memory>
#include <dinput.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <psapi.h>
#include <intrin.h>

#include <D3DX11tex.h>
#include <d3d11.h>
#include <dxgi.h>
#include "kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"

#include "Singleton.h"

// 进程和窗口的一些信息
#define GAME_WIN_CLASS "UnrealWindow"
#define GAME_WIN_NAME "POLYGON  "
#define PROCESS_NAME "POLYGON-Win64-Shipping.exe"
#define MODULE_NAME "POLYGON-Win64-Shipping.exe"

class Player;

using namespace std;

typedef HRESULT(__stdcall * Present)(IDXGISwapChain * pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK * WNDPROC)(HWND, UINT, WPARAM, LPARAM);

struct Vector2
{
	float x = 0.f, y = 0.f;
	Vector2()
	{
	}
	Vector2(float x, float y) : x(x), y(y)
	{
	}
};

struct Vector3
{
	float x = 0.f, y = 0.f, z = 0.f;
	Vector3()
	{
	}
	Vector3(float x, float y, float z) : x(x), y(y), z(z)
	{
	}
};

struct Vector4
{
	float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
	Vector4()
	{
	}
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
	{
	}
};

struct Rect
{
	float x = 0.f;
	float y = 0.f;
	float width = 0.f;
	float height = 0.f;
	float centerX = 0.f;
	float centerY = 0.f;

	Rect()
	{
	}
	Rect(float x, float y, float width, float height) : x(x), y(y), width(width), height(height)
	{
		centerX = width * 0.5f;
		centerY = height * 0.5f;
	}
};

// 摄像机矩阵
struct view_matrix_t
{
	float matrix[4][4];

	float * operator[](int index)
	{
		return matrix[index];
	}
};

// 人物坐标矩阵
struct transform_matrix_t
{
	float matrix[3][4];

	float * operator[](int index)
	{
		return matrix[index];
	}
};

// 人物transform
struct FTransform
{
	Vector4 rotation;
	Vector3 translation;
	Vector3 scale3D;
};

// DX9的D3DXMATRIX，自己定义一个来用
struct MyD3DXMATRIX
{
public:
	float _11;
	float _12;
	float _13;
	float _14;

	float _21;
	float _22;
	float _23;
	float _24;

	float _31;
	float _32;
	float _33;
	float _34;

	float _41;
	float _42;
	float _43;
	float _44;

	void matrixMultiply(MyD3DXMATRIX & out, const MyD3DXMATRIX & other)
	{
		out._11 = this->_11 * other._11 + this->_12 * other._21 + this->_13 * other._31 + this->_14 * other._41;
		out._12 = this->_11 * other._12 + this->_12 * other._22 + this->_13 * other._32 + this->_14 * other._42;
		out._13 = this->_11 * other._13 + this->_12 * other._23 + this->_13 * other._33 + this->_14 * other._43;
		out._14 = this->_11 * other._14 + this->_12 * other._24 + this->_13 * other._34 + this->_14 * other._44;
		out._21 = this->_21 * other._11 + this->_22 * other._21 + this->_23 * other._31 + this->_24 * other._41;
		out._22 = this->_21 * other._12 + this->_22 * other._22 + this->_23 * other._32 + this->_24 * other._42;
		out._23 = this->_21 * other._13 + this->_22 * other._23 + this->_23 * other._33 + this->_24 * other._43;
		out._24 = this->_21 * other._14 + this->_22 * other._24 + this->_23 * other._34 + this->_24 * other._44;
		out._31 = this->_31 * other._11 + this->_32 * other._21 + this->_33 * other._31 + this->_34 * other._41;
		out._32 = this->_31 * other._12 + this->_32 * other._22 + this->_33 * other._32 + this->_34 * other._42;
		out._33 = this->_31 * other._13 + this->_32 * other._23 + this->_33 * other._33 + this->_34 * other._43;
		out._34 = this->_31 * other._14 + this->_32 * other._24 + this->_33 * other._34 + this->_34 * other._44;
		out._41 = this->_41 * other._11 + this->_42 * other._21 + this->_43 * other._31 + this->_44 * other._41;
		out._42 = this->_41 * other._12 + this->_42 * other._22 + this->_43 * other._32 + this->_44 * other._42;
		out._43 = this->_41 * other._13 + this->_42 * other._23 + this->_43 * other._33 + this->_44 * other._43;
		out._44 = this->_41 * other._14 + this->_42 * other._24 + this->_43 * other._34 + this->_44 * other._44;
	}
};

// 骨骼数据
struct BoneData
{
	Vector2 head; // 头
	Vector2 neck; // 脖子
	Vector2 chest; // 胸部
	Vector2 leftShoulder; // 左肩
	Vector2 rightShoulder; // 右肩
	Vector2 leftElbow; // 左手肘
	Vector2 rightElbow; // 右手肘
	Vector2 leftWrist; // 左手腕
	Vector2 rightWrist; // 右手腕
	Vector2 pelvis; // 盆骨
	Vector2 leftThigh; // 左大腿
	Vector2 rightThigh; // 右大腿
	Vector2 leftKnee; // 左膝盖
	Vector2 rightKnee; // 右膝盖
	Vector2 leftAnkle; // 左脚腕
	Vector2 rightAnkle; // 右脚腕
};

class GlobalVars : public Singleton <GlobalVars>
{
public:
	// 游戏进程ID
	DWORD pId;
	// 游戏窗口的句柄
	HWND hWindow;

	// 游戏模块基址
	uintptr_t baseAddr;
	// 世界地址
	uintptr_t worldAddr;
	// 摄像机矩阵地址
	uintptr_t viewMatrixAddr;

	// ULevel地址
	uintptr_t uLevelAddr;
	// Actor数量
	int actorCount;
	// Actor数组地址
	uintptr_t actorsAddr;

	struct Offsets
	{
		DWORD world = 0x057ED6B0; // 世界基址
		DWORD gname = 0x569D350; // Gname基址

		DWORD viewMatrix = 0x054017F0; // 矩阵基址
		DWORD viewMatrix_offset1 = 0x20; // 矩阵偏移1
		DWORD viewMatrix_offset2 = 0x280; // 矩阵偏移2

		DWORD uLevel_offset = 0x30; // ULevel偏移
		DWORD actorCount_offset = 0xB0; // Count偏移
		DWORD actorArray_offset = 0xA8; // Actors数组偏移

		DWORD actorPosition_offset = 0x130; // 玩家坐标偏移
		DWORD actorPositionX_offset = 0x1D0; // 玩家X坐标偏移
		DWORD actorPositionY_offset = 0x1D4; // 玩家Y坐标偏移
		DWORD actorPositionZ_offset = 0x1D8; // 玩家Z坐标偏移

		DWORD playerSpeed_offset = 0x98; // 玩家速度偏移

		DWORD playerMesh_offset = 0x280; // 玩家网格偏移
		DWORD playerComponentToWorld_offset = 0x1C0; // 玩家阵列偏移
		DWORD playerBoneArray_offset = 0x4A0; // 玩家骨骼数组偏移



		DWORD playerHp_offset = 0x7C4; // 血量偏移
		DWORD playerFlag1_offset = 0x108; // 玩家类型偏移  10-人 15-枪 100-箱子
		DWORD playerFlag2_offset = 0x50; // 玩家类型偏移

		DWORD bullet = 0x02AEFFB8;
		DWORD bullet_offset1 = 0xF0;
		DWORD bullet_offset2 = 0x3D8;
		DWORD bullet_offset3 = 0x148; // 武器1，每个武器偏移8字节
		DWORD bullet_count_offset = 0x568; // 枪械结构里的子弹数量偏移
		DWORD gun_recoil_x_offset = 0x68; // 水平方向后坐力，在子弹数量偏移基础上增加偏移量
		DWORD gun_recoil_y_offset = 0x30; // 垂直方向后坐力，在子弹数量偏移基础上增加偏移量
		DWORD gun_jitter_offset = 0x28; // 枪械抖动，在子弹数量偏移基础上增加偏移量

	} ofs;

	// 窗口区域
	Rect drawRect;

	// 还活着的敌人数量，暂时把队友也放进去了，用标识来区分
	int activeEnemyCounter = 0;
	// 敌人数组
	shared_ptr<Player> enemyList[1000] = { 0 };
	// 自己
	shared_ptr<Player> localPlayer = nullptr;

	// 更新世界地址和矩阵地址
	void updateWorldAddrAndViewMatrixAddr();
	// 更新游戏窗口尺寸，也就是绘制区域
	void updateDrawRect();
	// 更新所有玩家信息
	void updatePlayerList();
	// 打印地址偏移
	void printOffsets();
};
