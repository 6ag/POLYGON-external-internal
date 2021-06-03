#pragma once
#include "Singleton.h"
#include "GlobalVars.h"
#include "Color.h"

class Renderer : public Singleton <Renderer>
{
public:
	~Renderer()
	{
	};
	void increaseSpeed();

	void imDrawInit();
	void drawFrames();

	void drawImText(const Vector2 & pos, const char * text, Color color, bool outline = false, float fontSize = 15.0f);
	void drawImLine(const Vector2 & p1, const Vector2 & p2, Color color, float thickness = 1.0f);
	void drawImCircle(const Vector2 & center, float radius, int numSegments, Color color, float thickness = 1.0f);
	void drawImRect(const Vector2 & pos, const Vector2 & size, Color color, float thickness = 1.0f);
	void drawImCircleFilled(const Vector2 & center, float radius, int numSegments, Color color);
	void drawImRectFilled(const Vector2 & pos, const Vector2 & size, Color color);

	ImU32 GetU32(Color _color)
	{
		return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8) + (_color[0] & 0xff);
	}

	// 绘制线条宽度
	const float thickness = 1.0f;
	// 绘制文字字体大小
	const int fontSize = 20;

	// dx11驱动
	ID3D11Device * pD3DDevice = nullptr;
	// dx11上下文
	ID3D11DeviceContext * pD3DDeviceContext = nullptr;
	ID3D11RenderTargetView * pMainRenderTargetView = nullptr;
	// dx交换链
	IDXGISwapChain * pSwapChain = nullptr;

	ImDrawList * pImBuffer = nullptr;
private:
	void baseAddrEsp(shared_ptr<Player> player);
	void noRecoil();
	bool playerWorldToScreen(shared_ptr<Player> player, view_matrix_t matrix);
	void boxEsp(shared_ptr<Player> player);
	void lineEsp(shared_ptr<Player> player);
	void hpEsp(shared_ptr<Player> player);
	void distanceEsp(shared_ptr<Player> player);

	void aimbotRangeEsp();
	void aimbotArrowEsp(shared_ptr<Player> player);
	void aimbot(shared_ptr<Player> player);
	void aimAt(Vector2 targetPos);
	void drawTest(shared_ptr<Player> player, view_matrix_t matrix, Color color);
	void drawMatchstickMen(shared_ptr<Player> player, view_matrix_t matrix, Color color);
	bool aimbootWorldToScreen(const Vector2 & screen_size, const Vector3 & pos, Vector2 & retPos, view_matrix_t matrix);
	Vector3 getBonePos(uintptr_t skeletonMatrixAddr, uintptr_t boneAddr);
	bool boneWorldToScreen(const Vector2 & screen_size, const Vector3 & pos, Vector2 & retPos, view_matrix_t matrix);
	void readTransform(FTransform & out, uintptr_t addr);
	void toMatrixWithScale(MyD3DXMATRIX & out, Vector4 rotation, Vector3 translation, Vector3 scale3D);
	bool boneCheckPlayerActive(shared_ptr<Player> player, view_matrix_t matrix);

	struct vertex
	{
		float x, y, z, rhw;
		D3DCOLOR color;
	};

	// 自瞄锁定目标
	shared_ptr<Player> lockAimTarget = nullptr;
	// 优化点按瞄准镜切换
	int aimCounter = 0;
};
