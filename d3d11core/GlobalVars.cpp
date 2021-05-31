#include "GlobalVars.h"
#include "Memory.h"
#include "Renderer.h"
#include "Player.h"

void GlobalVars::updateWorldAddrAndViewMatrixAddr()
{
	// 获取世界地址
	GlobalVars::get().worldAddr = Memory::get().read<uintptr_t>(GlobalVars::get().baseAddr + GlobalVars::get().ofs.world);

	// 获取矩阵地址
	uintptr_t viewMatrixBaseAddr = Memory::get().read<uintptr_t>(GlobalVars::get().baseAddr + GlobalVars::get().ofs.viewMatrix);
	uintptr_t viewMatrixBaseAddr_offset1 = Memory::get().read<uintptr_t>(viewMatrixBaseAddr + GlobalVars::get().ofs.viewMatrix_offset1);
	GlobalVars::get().viewMatrixAddr = viewMatrixBaseAddr_offset1 + GlobalVars::get().ofs.viewMatrix_offset2;
	/*cout << "世界地址 = " << GlobalVars::get().worldAddr << endl;
	cout << "矩阵地址 = " << GlobalVars::get().viewMatrixAddr << endl;*/
}

void GlobalVars::updateDrawRect()
{
	RECT gameWindowRect;
	POINT point = { 0 };
	GetClientRect(GlobalVars::get().hWindow, &gameWindowRect);
	ClientToScreen(GlobalVars::get().hWindow, &point);

	GlobalVars::get().drawRect = Rect(gameWindowRect.left,
									  gameWindowRect.top,
									  gameWindowRect.right,
									  gameWindowRect.bottom);
	/*cout << "游戏窗口.x = " << GlobalVars::get().drawRect.x << endl;
	cout << "游戏窗口.y = " << GlobalVars::get().drawRect.y << endl;
	cout << "游戏窗口.width = " << GlobalVars::get().drawRect.width << endl;
	cout << "游戏窗口.height = " << GlobalVars::get().drawRect.height << endl;*/
}

void GlobalVars::updatePlayerList()
{
	GlobalVars::get().uLevelAddr = Memory::get().read<uintptr_t>(GlobalVars::get().worldAddr + GlobalVars::get().ofs.uLevel_offset);
	GlobalVars::get().actorCount = Memory::get().read<int>(GlobalVars::get().uLevelAddr + GlobalVars::get().ofs.actorCount_offset);
	GlobalVars::get().actorsAddr = Memory::get().read<uintptr_t>(GlobalVars::get().uLevelAddr + GlobalVars::get().ofs.actorArray_offset);

	/*cout << "uLevelAddr = " << GlobalVars::get().uLevelAddr << endl;
	cout << "actorCount = " << GlobalVars::get().actorCount << endl;
	cout << "actorsAddr = " << GlobalVars::get().actorsAddr << endl;*/

	playerList.clear();
	for (int i = 0; i < GlobalVars::get().actorCount; i++)
	{
		uintptr_t actorBaseAddr = Memory::get().read<uintptr_t>(GlobalVars::get().actorsAddr + i * 0x8);
		shared_ptr<Player> player = make_shared<Player>(actorBaseAddr);
		player->update();

		/*if (player->hp < 1)
		{
			continue;
		}*/

		// 区分自己和其他人
		if (player->type == PlayerType::other)
		{
			playerList.push_back(player);
		}
		if (player->type == PlayerType::oneself)
		{
			localPlayer = player;
		}
	}

	// 区分敌人和队友
	for (int i = 0; i < playerList.size(); i++)
	{
		if (localPlayer == nullptr || playerList[i] == nullptr)
		{
			continue;
		}

		if (playerList[i]->bpCName == localPlayer->bpCName)
		{
			playerList[i]->type = PlayerType::team;
		}
		else
		{
			playerList[i]->type = PlayerType::enemy;
		}
	}
}

void GlobalVars::printOffsets()
{
	Offsets * base = &ofs;
	cout << base << " = link to struct of offsets { \n\n";
	for (int i = 0; i < sizeof(Offsets) / sizeof(DWORD); i++)
	{
		cout << hex << "0x" << *(uintptr_t *)((uintptr_t)base + i * sizeof(DWORD)) << dec << "\n";
	}
	cout << "} \n";
}
