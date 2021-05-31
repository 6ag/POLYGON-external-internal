#include "Player.h"
#include "Memory.h"
#include "GlobalVars.h"

using namespace std;

Player::Player(uintptr_t addres)
{
	base = addres;
}

Player::~Player()
{

}

// BP_PG_Character_Blue_C
// BP_PG_Character_Red_C
// 读取name
std::string Player::getBpCName()
{
	int objId = Memory::get().read<int>(base + 0x18);
	UCHAR tableLocaltion = (UINT)(int)(objId >> 16);
	ULONG rowLocaltion = (USHORT)objId;
	int pRowLocation = rowLocaltion;
	//cout << "rowLocaltion=" << rowLocaltion << ",pRowLocation=" << pRowLocation << endl;

	uintptr_t gNameTable = GlobalVars::get().baseAddr + GlobalVars::get().ofs.gname;
	uintptr_t tableLocationAddress = Memory::get().read<uintptr_t>(gNameTable + tableLocaltion * 8);
	tableLocationAddress += 2 * pRowLocation;
	USHORT sLength = Memory::get().read<USHORT>(tableLocationAddress);
	sLength = sLength >> 6;
	//cout << "sLength=" << sLength << endl;
	if (sLength < 128)
	{
		char buffer[128] = { '\0' };
		for (int i = 0; i < sLength; i++)
		{
			buffer[i] = Memory::get().read<char>(tableLocationAddress + 2 + i);
		}

		std::string name = buffer;
		name.resize(sLength);
		//cout << "objId=" << objId << ",address=" << tableLocationAddress + 2 << ",length=" << sLength << ",buffer=" << buffer << ",name=" << name << endl;
		return name;
	}
	return "None";
}

void Player::update()
{
	bpCName = getBpCName();

	// 不属于双方阵营直接过滤
	if (bpCName != "BP_PG_Character_Blue_C" && bpCName != "BP_PG_Character_Red_C")
	{
		return;
	}

	int flag1 = Memory::get().read<int>(base + 0x40); // 0x40 1-人 0-未知
	int flag2 = Memory::get().read<int>(base + 0x1A8); // 0x1A8 25-自己 24-其他人
	int flag3 = Memory::get().read<int>(base + 0x2F0); // 0x2F0 0-自己

	// 自己 0x40-1 0x1A8-25 0x2F0-0
	if (flag1 == 1 && flag2 == 25 && flag3 == 0)
	{
		//cout << "x=" << origin.x << " y=" << origin.y << " z=" << origin.z << endl;
		type = PlayerType::oneself;
	}
	else
	{
		type = PlayerType::other;
	}

	// 不是未知类型才去读取其他数据
	if (type != PlayerType::none)
	{
		// HP
		/*hp = Memory::get().read<float>(base + GlobalVars::get().ofs.playerHp_offset);
		if (hp < 1)
		{
			return;
		}*/

		// 注意：第一人称游戏有时候看不到自己身上的一些地址，可以调整坐标，让地址显示出来。0-200调整
		float offsetX = 0;

		// 坐标
		uintptr_t playerOriginAddr = Memory::get().read<uintptr_t>(base + GlobalVars::get().ofs.actorPosition_offset);
		origin.x = Memory::get().read<float>(playerOriginAddr + GlobalVars::get().ofs.actorPositionX_offset) + offsetX;
		origin.y = Memory::get().read<float>(playerOriginAddr + GlobalVars::get().ofs.actorPositionY_offset);
		origin.z = Memory::get().read<float>(playerOriginAddr + GlobalVars::get().ofs.actorPositionZ_offset);

		// 骨骼
		uintptr_t meshAddr = Memory::get().read<uintptr_t>(base + GlobalVars::get().ofs.playerMesh_offset);
		skeletonArrayAddr = Memory::get().read<uintptr_t>(meshAddr + GlobalVars::get().ofs.playerBoneArray_offset);
		skeletonMatrixAddr = meshAddr + GlobalVars::get().ofs.playerComponentToWorld_offset;
	}
}
