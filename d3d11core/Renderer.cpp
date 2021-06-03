#include "Renderer.h"
#include "Memory.h"
#include "Menu.h"
#include "Player.h"

void Renderer::drawFrames()
{
	Menu::get().imGuiStart();
	float minCrossCenter = 99999999.0f;

	view_matrix_t matrix = Memory::get().read<view_matrix_t>(GlobalVars::get().viewMatrixAddr);

	// 临时的最佳自瞄目标
	shared_ptr<Player> bestAimTarget = nullptr;

	/*if (GlobalVars::get().localPlayer != nullptr && playerWorldToScreen(GlobalVars::get().localPlayer, matrix))
	{
		baseAddrEsp(GlobalVars::get().localPlayer);
	}*/

	//cout << GlobalVars::get().activeEnemyCounter << endl;
	for (int i = 0; i < GlobalVars::get().playerList.size(); i++)
	{
		if (playerWorldToScreen(GlobalVars::get().playerList[i], matrix))
		{
			// 通过骨骼去计算玩家是否是活的
			if (!boneCheckPlayerActive(GlobalVars::get().playerList[i], matrix)/*GlobalVars::get().playerList[i]->hp < 1*/)
			{
				continue;
			}

			//baseAddrEsp(GlobalVars::get().playerList[i]);

			// 透视
			if (GlobalVars::get().playerList[i]->distance <= Menu::get().espDistance)
			{
				// 方框透视
				if (Menu::get().boxEsp)
				{
					boxEsp(GlobalVars::get().playerList[i]);
				}

				// 连线透视
				if (Menu::get().lineEsp)
				{
					lineEsp(GlobalVars::get().playerList[i]);
				}

				// 血量透视
				if (Menu::get().hpEsp)
				{
					hpEsp(GlobalVars::get().playerList[i]);
				}

				// 距离透视
				if (Menu::get().distanceEsp)
				{
					distanceEsp(GlobalVars::get().playerList[i]);
				}

				// 骨骼透视
				if (Menu::get().boneEsp)
				{
					if (GlobalVars::get().playerList[i]->type == PlayerType::enemy)
					{
						drawMatchstickMen(GlobalVars::get().playerList[i], matrix, Menu::get().espColor);
					}
					else if (Menu::get().openFriendEsp)
					{
						drawMatchstickMen(GlobalVars::get().playerList[i], matrix, Color::Green);
					}
				}
			}

			// 自瞄
			if (Menu::get().aimbot && GlobalVars::get().playerList[i]->type == PlayerType::enemy && (GlobalVars::get().playerList[i]->distance > 5 && GlobalVars::get().playerList[i]->distance <= Menu::get().aimbotDistance) && lockAimTarget == nullptr)
			{
				// 准星距离，目标距离准星的距离，取所有目标中距离准星最小的。还有一种筛选自瞄目标的方式是取所有目标距离自己最近的。
				float xDiff = GlobalVars::get().drawRect.centerX - GlobalVars::get().playerList[i]->box.centerX;
				float yDiff = GlobalVars::get().drawRect.centerY - GlobalVars::get().playerList[i]->box.centerY;
				float crossCenter = sqrt(pow(xDiff, 2) + pow(yDiff, 2));
				if (crossCenter < Menu::get().aimbotRadius && crossCenter < minCrossCenter)
				{
					minCrossCenter = crossCenter;
					bestAimTarget = GlobalVars::get().playerList[i];
				}

				// 绘制目标离准星的距离，用于测试
				/*if (true)
				{
					char text[50];
					sprintf_s(text, "%.0f", crossCenter);
					drawImText(Vector2(GlobalVars::get().playerList[i]->box.x, GlobalVars::get().playerList[i]->box.y + GlobalVars::get().playerList[i]->box.height), text, Menu::get().espColor);
				}*/
			}
		}
	}

	if (Menu::get().aimbot)
	{
		aimbotRangeEsp();
	}

	if (Menu::get().aimbot && bestAimTarget != nullptr)
	{
		// 锁定自瞄目标
		lockAimTarget = bestAimTarget;

		// 把自瞄对象标记出来
		aimbotArrowEsp(lockAimTarget);
	}

	// 自瞄
	if (Menu::get().aimbot && GetAsyncKeyState(VK_RBUTTON) == -32768)
	{
		if (lockAimTarget != nullptr)
		{
			// 防止切换瞄准镜也会自瞄
			aimCounter++;
			if (aimCounter > 15)
			{
				// 通过骨骼去计算玩家是否是活的，如果能找到血量，用血量判断更好
				if (boneCheckPlayerActive(lockAimTarget, matrix)/*lockAimTarget->hp > 1*/)
				{
					aimbot(lockAimTarget);
				}
				else
				{
					aimCounter = 0;
					lockAimTarget = nullptr;
				}
			}
		}
	}
	else
	{
		aimCounter = 0;
		lockAimTarget = nullptr;
	}

	// 无后座
	if (Menu::get().noRecoil || Menu::get().lockBullet)
	{
		noRecoil();
	}

	Menu::get().imGuiEnd();
}

static map<uintptr_t, int> baseMap;

// 基址透视，用于测试
void Renderer::baseAddrEsp(shared_ptr<Player> player)
{
	if (player->base == 0)
	{
		return;
	}
	char text[50];
	sprintf_s(text, "0x%llX", player->base);

	// counter 1-6
	static int counter = 0;
	if (counter == 6)
	{
		counter = 0;
	}
	counter++;
	if (baseMap[player->base] < 1)
	{
		baseMap[player->base] = counter;
	}

	Rect tmpBox = player->box;
	// 防止有些小东西计算出来宽度高度0
	/*tmpBox.width = max(tmpBox.width, 100);
	tmpBox.height = max(tmpBox.height, 200);*/

	// 地址上下动
	//tmpBox.y += 150 * cos(counter);

	Color color = Color::Red;
	switch (baseMap[player->base])
	{
		case 1:
			color = Color::White;
			break;
		case 2:
			color = Color::Red;
			break;
		case 3:
			color = Color::Green;
			break;
		case 4:
			color = Color::Orange;
			break;
		case 5:
			color = Color::Yellow;
			break;
		case 6:
			color = Color::Blue;
			break;
		default:
			cout << "代码肯定有问题" << endl;
			break;
	}

	//drawImText(Vector2(tmpBox.centerX, tmpBox.y), player->bpCName.c_str(), color, false, 25);
	drawImText(Vector2(tmpBox.centerX, tmpBox.y), text, color, false, 25);
	drawImRect(Vector2(player->box.x, player->box.y), Vector2(player->box.width, player->box.height), color);
	lineEsp(player);

	/*char hp[50];
	sprintf_s(hp, "%.0f", player->hp);
	drawImText(Vector2(tmpBox.centerX, tmpBox.y), hp, color, false, 25);*/
}

// 骨骼检测是否死亡
bool Renderer::boneCheckPlayerActive(shared_ptr<Player> player, view_matrix_t matrix)
{
	BoneData boneData;
	Vector2 screenSize = Vector2(GlobalVars::get().drawRect.width, GlobalVars::get().drawRect.height);

	if (boneWorldToScreen(screenSize, getBonePos(player->skeletonMatrixAddr, player->skeletonArrayAddr + 5 * 48), boneData.head, matrix))
	{
		if (boneWorldToScreen(screenSize, getBonePos(player->skeletonMatrixAddr, player->skeletonArrayAddr + 1 * 48), boneData.pelvis, matrix))
		{
			if (boneData.head.x != boneData.pelvis.x && boneData.head.y != boneData.pelvis.y)
			{
				// 没死
				return true;
			}
		}
	}

	// 人死了
	return false;
}

// 人物加速
void Renderer::increaseSpeed()
{
	Memory::get().write<float>(GlobalVars::get().localPlayer->base + GlobalVars::get().ofs.playerSpeed_offset, Menu::get().moveSpeed);
}

// 子弹锁定和无后座力
void Renderer::noRecoil()
{
	// 057E0360 -> 0 -> A0 -> 580 -> 128 -> 300 第一把枪子弹数量 int
	// 057E0360 -> 0 -> A0 -> 580 -> 130 -> 300 第二把枪子弹数量 int
	// 057D5EF0 -> 30 -> 250 -> 580 -> 128 -> 2C0 第一把枪射速，越小越快。float
	// 057D5EF0 -> 30 -> 250 -> 580 -> 130 -> 2C0 第一把枪射速，越小越快。float
	// 057D5EF0 -> 30 -> 260 -> 580 -> 128 -> 2C8 第一把枪后坐力，设为0没后坐力。float
	// 057D5EF0 -> 30 -> 260 -> 580 -> 130 -> 2C8 第一把枪后坐力，设为0没后坐力。float

	// 子弹锁定
	if (Menu::get().lockBullet)
	{
		uintptr_t bulletBaseAddr = Memory::get().read<uintptr_t>(GlobalVars::get().baseAddr + 0x057E0360);
		uintptr_t bulletOffset1 = Memory::get().read<uintptr_t>(bulletBaseAddr);
		uintptr_t bulletOffset2 = Memory::get().read<uintptr_t>(bulletOffset1 + 0xA0);
		uintptr_t bulletOffset3 = Memory::get().read<uintptr_t>(bulletOffset2 + 0x580);
		uintptr_t bulletOffset4_1 = Memory::get().read<uintptr_t>(bulletOffset3 + 0x128);
		uintptr_t bulletOffset4_2 = Memory::get().read<uintptr_t>(bulletOffset3 + 0x130);

		Memory::get().write<int>(bulletOffset4_1 + 0x300, 30);
		Memory::get().write<int>(bulletOffset4_2 + 0x300, 7);
	}

	// 无后坐力
	if (Menu::get().noRecoil)
	{
		uintptr_t gunBaseAddr = Memory::get().read<uintptr_t>(GlobalVars::get().baseAddr + 0x057D5EF0);
		uintptr_t gunFiringSpeedOffset1 = Memory::get().read<uintptr_t>(gunBaseAddr + 0x30);
		uintptr_t gunFiringSpeedOffset2 = Memory::get().read<uintptr_t>(gunFiringSpeedOffset1 + 0x250);
		uintptr_t gunFiringSpeedOffset3 = Memory::get().read<uintptr_t>(gunFiringSpeedOffset2 + 0x580);
		uintptr_t gunFiringSpeedOffset4_1 = Memory::get().read<uintptr_t>(gunFiringSpeedOffset3 + 0x128);
		uintptr_t gunFiringSpeedOffset4_2 = Memory::get().read<uintptr_t>(gunFiringSpeedOffset3 + 0x130);

		uintptr_t gunNorecoilOffset2 = Memory::get().read<uintptr_t>(gunFiringSpeedOffset1 + 0x260);
		uintptr_t gunNorecoilOffset3 = Memory::get().read<uintptr_t>(gunNorecoilOffset2 + 0x580);
		uintptr_t gunNorecoilOffset4_1 = Memory::get().read<uintptr_t>(gunNorecoilOffset3 + 0x128);
		uintptr_t gunNorecoilOffset4_2 = Memory::get().read<uintptr_t>(gunNorecoilOffset3 + 0x130);

		Memory::get().write<float>(gunNorecoilOffset4_1 + 0x2C8, 0.0f);
		Memory::get().write<float>(gunNorecoilOffset4_2 + 0x2C8, 0.0f);

		// 射速
		Memory::get().write<float>(gunFiringSpeedOffset4_1 + 0x2C0, Menu::get().fireSpeed);
		Memory::get().write<float>(gunFiringSpeedOffset4_2 + 0x2C0, Menu::get().fireSpeed);
	}
}

bool Renderer::playerWorldToScreen(shared_ptr<Player> player, view_matrix_t matrix)
{
	float w = matrix[0][3] * player->origin.x + matrix[1][3] * player->origin.y + matrix[2][3] * player->origin.z + matrix[3][3];
	if (w < 0.001f)
		return false;

	// 目标和摄像机的距离
	player->distance = w / 100.0f;

	float centerX = GlobalVars::get().drawRect.centerX + (matrix[0][0] * player->origin.x + matrix[1][0] * player->origin.y + matrix[2][0] * player->origin.z + matrix[3][0]) / w * GlobalVars::get().drawRect.centerX;
	float minY = GlobalVars::get().drawRect.centerY - (matrix[0][1] * player->origin.x + matrix[1][1] * player->origin.y + matrix[2][1] * (player->origin.z + 110) + matrix[3][1]) / w * GlobalVars::get().drawRect.centerY;
	float maxY = GlobalVars::get().drawRect.centerY - (matrix[0][1] * player->origin.x + matrix[1][1] * player->origin.y + matrix[2][1] * (player->origin.z - 110) + matrix[3][1]) / w * GlobalVars::get().drawRect.centerY;
	// 为了模糊计算高度，这里把z坐标上下移动了100，来大概估算出人物高度

	player->box.height = maxY - minY;
	player->box.width = player->box.height * .5f;
	player->box.x = centerX - player->box.width * .5f;
	player->box.y = minY;
	player->box.centerX = centerX;
	player->box.centerY = minY + player->box.height * .5f;

	return true;
}

void Renderer::aimbotRangeEsp()
{
	drawImCircle(GlobalVars::get().drawRect.getCenter(), Menu::get().aimbotRadius, 100, Color::White);
	if (Menu::get().aimCross)
	{
		drawImCircle(GlobalVars::get().drawRect.getCenter(), 15, 100, Color::White);
		drawImLine(Vector2(GlobalVars::get().drawRect.centerX - 20, GlobalVars::get().drawRect.centerY),
				   Vector2(GlobalVars::get().drawRect.centerX - 10, GlobalVars::get().drawRect.centerY),
				   Color::White);

		drawImLine(Vector2(GlobalVars::get().drawRect.centerX + 20, GlobalVars::get().drawRect.centerY),
				   Vector2(GlobalVars::get().drawRect.centerX + 10, GlobalVars::get().drawRect.centerY),
				   Color::White);

		drawImLine(Vector2(GlobalVars::get().drawRect.centerX, GlobalVars::get().drawRect.centerY - 20),
				   Vector2(GlobalVars::get().drawRect.centerX, GlobalVars::get().drawRect.centerY - 10),
				   Color::White);

		drawImLine(Vector2(GlobalVars::get().drawRect.centerX, GlobalVars::get().drawRect.centerY + 20),
				   Vector2(GlobalVars::get().drawRect.centerX, GlobalVars::get().drawRect.centerY + 10),
				   Color::White);
	}
}

void Renderer::aimbotArrowEsp(shared_ptr<Player> player)
{
	Color arrowColor = Color::Orange;
	//Color arrowColor = Menu::get().espColor;
	drawImLine(Vector2(player->box.centerX - player->box.width * 0.5f, player->box.y - player->box.height * 0.5f),
			   Vector2(player->box.centerX, player->box.y),
			   arrowColor,
			   1.5f);
	drawImLine(Vector2(player->box.centerX + player->box.width * 0.5f, player->box.y - player->box.height * 0.5f),
			   Vector2(player->box.centerX, player->box.y),
			   arrowColor,
			   1.5f);
	drawImLine(Vector2(player->box.centerX, player->box.y - player->box.height * 1.5f),
			   Vector2(player->box.centerX, player->box.y),
			   arrowColor,
			   1.5f);
}

void Renderer::boxEsp(shared_ptr<Player> player)
{
	float thikness = player->distance <= 100.f ? thikness = 1.f : thikness = 0.5f;
	if (player->type == PlayerType::enemy)
	{
		drawImRect(Vector2(player->box.x, player->box.y), Vector2(player->box.width, player->box.height), Menu::get().espColor, thikness);
	}
	else if (Menu::get().openFriendEsp)
	{
		drawImRect(Vector2(player->box.x, player->box.y), Vector2(player->box.width, player->box.height), Color::Green, thikness);
	}
}

void Renderer::lineEsp(shared_ptr<Player> player)
{
	float thikness = player->distance <= 100.f ? thikness = 1.f : thikness = 0.5f;
	if (player->type == PlayerType::enemy)
	{
		drawImLine(Vector2(GlobalVars::get().drawRect.centerX, 0), Vector2(player->box.centerX, player->box.y), Menu::get().espColor, thikness);
	}
	else if (Menu::get().openFriendEsp)
	{
		drawImLine(Vector2(GlobalVars::get().drawRect.centerX, 0), Vector2(player->box.centerX, player->box.y), Color::Green, thikness);
	}
}

void Renderer::hpEsp(shared_ptr<Player> player)
{
	double scale = (100.f / player->hp);
	float height = player->box.height / scale;
	float width = 1.5f + (player->distance / sqrt(player->distance) / 25);
	if (player->distance <= 500.f)
	{
		width = 4;
	}
	width = min(width, 4);

	int green = (1.f / scale * 255) * 0.7f;
	int red = (100 - player->hp) * 3.2f;
	if (red > 255) red = 255;

	drawImRectFilled(Vector2(player->box.x - width * 2, player->box.y), Vector2(width, height), Color(red / 255.0f, green / 255.0f, 0.0f));
	drawImRect(Vector2(player->box.x - width * 2, player->box.y), Vector2(width + 1, player->box.height), Color::Black);
}

void Renderer::distanceEsp(shared_ptr<Player> player)
{
	char text[25];
	sprintf_s(text, "%.0fm", player->distance);
	if (player->type == PlayerType::enemy)
	{
		drawImText(Vector2(player->box.x, player->box.y + player->box.height), text, Menu::get().espColor);
	}
	else if (Menu::get().openFriendEsp)
	{
		drawImText(Vector2(player->box.x, player->box.y + player->box.height), text, Color::Green);
	}
}

void Renderer::aimbot(shared_ptr<Player> player)
{
	view_matrix_t matrix = Memory::get().read<view_matrix_t>(GlobalVars::get().viewMatrixAddr);
	uintptr_t meshAddr = Memory::get().read<uintptr_t>(player->base + GlobalVars::get().ofs.playerMesh_offset);
	uintptr_t skeletonArrayAddr = Memory::get().read<uintptr_t>(meshAddr + GlobalVars::get().ofs.playerBoneArray_offset);
	uintptr_t skeletonMatrixAddr = meshAddr + GlobalVars::get().ofs.playerComponentToWorld_offset;

	Vector2 screenSize = GlobalVars::get().drawRect.getSize();
	Vector2 bone2D;

	int index = 5;
	if (Menu::get().aimbotType == 0)
	{
		index = 5;
	}
	else if (Menu::get().aimbotType == 1)
	{
		index = 3;
	}
	if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + index * 48), bone2D, matrix))
	{
		/*cout << "base=" << player->base << ",index=" << index << ",bone2D.x=" << bone2D.x << ",bone2D.y=" << bone2D.x << endl;
		cout << "centerX=" << GlobalVars::get().drawRect.centerX << ",centerY=" << GlobalVars::get().drawRect.centerY << endl;*/
		// 缩放率越大，移动越平滑，但太过大会移动缓慢。缩放率越小，移动就越快速，加速度可能还会让镜头甩动
		//float scaleRate = 6.0f;
		//mouse_event(MOUSEEVENTF_MOVE, (bone2D.x - GlobalVars::get().drawRect.centerX) / scaleRate, (bone2D.y - GlobalVars::get().drawRect.centerY) / scaleRate, 0, 0);

		aimAt(bone2D);
	}
}

// 瞄准目标
void Renderer::aimAt(Vector2 targetPos)
{
	// 瞄准速度
	int aimSpeed = 12;
	float screenCenterX = GlobalVars::get().drawRect.centerX;
	float screenCenterY = GlobalVars::get().drawRect.centerY;
	float targetX = 0;
	float targetY = 0;

	if (targetPos.x != 0)
	{
		if (targetPos.x > screenCenterX)
		{
			targetX = -(screenCenterX - targetPos.x);
			targetX /= aimSpeed;
			if (targetX + screenCenterX > screenCenterX * 2) targetX = 0;
		}

		if (targetPos.x < screenCenterX)
		{
			targetX = targetPos.x - screenCenterX;
			targetX /= aimSpeed;
			if (targetX + screenCenterX < 0) targetX = 0;
		}
	}

	if (targetPos.y != 0)
	{
		if (targetPos.y > screenCenterY)
		{
			targetY = -(screenCenterY - targetPos.y);
			targetY /= aimSpeed;
			if (targetY + screenCenterY > screenCenterY * 2) targetY = 0;
		}

		if (targetPos.y < screenCenterY)
		{
			targetY = targetPos.y - screenCenterY;
			targetY /= aimSpeed;
			if (targetY + screenCenterY < 0) targetY = 0;
		}
	}
	mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(targetX), static_cast<DWORD>(targetY), NULL, NULL);
}

// 绘制火柴人
void Renderer::drawMatchstickMen(shared_ptr<Player> player, view_matrix_t matrix, Color color)
{
	uintptr_t skeletonMatrixAddr = player->skeletonMatrixAddr;
	uintptr_t skeletonArrayAddr = player->skeletonArrayAddr;
	BoneData boneData;
	Vector2 screenSize = Vector2(GlobalVars::get().drawRect.width, GlobalVars::get().drawRect.height);

	if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 5 * 48), boneData.head, matrix))
	{
		if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 3 * 48), boneData.chest, matrix))
		{
			if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 1 * 48), boneData.pelvis, matrix))
			{
				if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 10 * 48), boneData.leftShoulder, matrix))
				{
					if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 25 * 48), boneData.rightShoulder, matrix))
					{
						if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 11 * 48), boneData.leftElbow, matrix))
						{
							if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 26 * 48), boneData.rightElbow, matrix))
							{
								if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 12 * 48), boneData.leftWrist, matrix))
								{
									if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 27 * 48), boneData.rightWrist, matrix))
									{
										if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 44 * 48), boneData.leftThigh, matrix))
										{
											if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 39 * 48), boneData.rightThigh, matrix))
											{
												if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 45 * 48), boneData.leftKnee, matrix))
												{
													if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 40 * 48), boneData.rightKnee, matrix))
													{
														if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 46 * 48), boneData.leftAnkle, matrix))
														{
															if (boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + 41 * 48), boneData.rightAnkle, matrix))
															{
																drawImCircle(boneData.head, player->box.width * 0.15f, player->box.width * 0.2f, color);
																drawImLine(boneData.chest, boneData.pelvis, color);
																drawImLine(boneData.chest, boneData.leftShoulder, color);
																drawImLine(boneData.chest, boneData.rightShoulder, color);
																drawImLine(boneData.leftShoulder, boneData.leftElbow, color);
																drawImLine(boneData.rightShoulder, boneData.rightElbow, color);
																drawImLine(boneData.pelvis, boneData.leftThigh, color);
																drawImLine(boneData.pelvis, boneData.rightThigh, color);
																drawImLine(boneData.leftElbow, boneData.leftWrist, color);
																drawImLine(boneData.rightElbow, boneData.rightWrist, color);
																drawImLine(boneData.leftThigh, boneData.leftKnee, color);
																drawImLine(boneData.rightThigh, boneData.rightKnee, color);
																drawImLine(boneData.leftKnee, boneData.leftAnkle, color);
																drawImLine(boneData.rightKnee, boneData.rightAnkle, color);

																/*cout << "head=" << boneData.head.x << "," << boneData.head.y << endl;
																cout << "chest=" << boneData.chest.x << "," << boneData.chest.y << endl;
																cout << "pelvis=" << boneData.pelvis.x << "," << boneData.pelvis.y << endl;
																cout << "leftShoulder=" << boneData.leftShoulder.x << "," << boneData.leftShoulder.y << endl;
																cout << "rightShoulder=" << boneData.rightShoulder.x << "," << boneData.rightShoulder.y << endl;
																cout << "leftElbow=" << boneData.leftElbow.x << "," << boneData.leftElbow.y << endl;
																cout << "rightElbow=" << boneData.rightElbow.x << "," << boneData.rightElbow.y << endl;
																cout << "leftWrist=" << boneData.leftWrist.x << "," << boneData.leftWrist.y << endl;
																cout << "rightWrist=" << boneData.rightWrist.x << "," << boneData.rightWrist.y << endl;
																cout << "leftThigh=" << boneData.leftThigh.x << "," << boneData.leftThigh.y << endl;
																cout << "rightThigh=" << boneData.rightThigh.x << "," << boneData.rightThigh.y << endl;
																cout << "leftKnee=" << boneData.leftKnee.x << "," << boneData.leftKnee.y << endl;
																cout << "rightKnee=" << boneData.rightKnee.x << "," << boneData.rightKnee.y << endl;
																cout << "leftAnkle=" << boneData.leftAnkle.x << "," << boneData.leftAnkle.y << endl;
																cout << "rightAnkle=" << boneData.rightAnkle.x << "," << boneData.rightAnkle.y << endl;*/
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// 绘制测试骨骼
void Renderer::drawTest(shared_ptr<Player> player, view_matrix_t matrix, Color color)
{
	uintptr_t skeletonMatrixAddr = player->skeletonMatrixAddr;
	uintptr_t skeletonArrayAddr = player->skeletonArrayAddr;
	Vector2 screenSize = Vector2(GlobalVars::get().drawRect.width, GlobalVars::get().drawRect.height);
	Vector2 bone2D;
	for (int i = 0; i < 200; i++)
	{
		bool result = boneWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + i * 48), bone2D, matrix);
		if (result)
		{
			// 绘制文本来测试
			char text[5];
			sprintf_s(text, "%d", i);
			drawImText(bone2D, text, Menu::get().espColor);
		}
	}
}

// 骨骼坐标处理
Vector3 Renderer::getBonePos(uintptr_t skeletonMatrixAddr, uintptr_t boneAddr)
{
	FTransform bone;
	FTransform actor;
	MyD3DXMATRIX boneMatrix;
	MyD3DXMATRIX componentToWorldMatrix;
	MyD3DXMATRIX newMatrix;
	Vector3 retPos;

	readTransform(bone, boneAddr);
	readTransform(actor, skeletonMatrixAddr);

	toMatrixWithScale(boneMatrix, bone.rotation, bone.translation, bone.scale3D);
	toMatrixWithScale(componentToWorldMatrix, actor.rotation, actor.translation, actor.scale3D);

	boneMatrix.matrixMultiply(newMatrix, componentToWorldMatrix);

	retPos.x = newMatrix._41;
	retPos.y = newMatrix._42;
	retPos.z = newMatrix._43;

	return retPos;
}

// 读取transform信息
void Renderer::readTransform(FTransform & out, uintptr_t addr)
{
	transform_matrix_t transformMatrix = Memory::get().read<transform_matrix_t>(addr);

	out.rotation.x = transformMatrix[0][0];
	out.rotation.y = transformMatrix[0][1];
	out.rotation.z = transformMatrix[0][2];
	out.rotation.w = transformMatrix[0][3];

	out.translation.x = transformMatrix[1][0];
	out.translation.y = transformMatrix[1][1];
	out.translation.z = transformMatrix[1][2];

	out.scale3D.x = transformMatrix[2][0];
	out.scale3D.y = transformMatrix[2][1];
	out.scale3D.z = transformMatrix[2][2];
}

// 计算骨骼坐标差
void Renderer::toMatrixWithScale(MyD3DXMATRIX & out, Vector4 rotation, Vector3 translation, Vector3 scale3D)
{
	float x2;
	float y2;
	float z2;
	float xx2;
	float yy2;
	float zz2;
	float yz2;
	float wx2;
	float xy2;
	float wz2;
	float xz2;
	float wy2;

	out._41 = translation.x;
	out._42 = translation.y;
	out._43 = translation.z;

	x2 = rotation.x + rotation.x;
	y2 = rotation.y + rotation.y;
	z2 = rotation.z + rotation.z;

	xx2 = rotation.x * x2;
	yy2 = rotation.y * y2;
	zz2 = rotation.z * z2;
	out._11 = (1 - (yy2 + zz2)) * scale3D.x;
	out._22 = (1 - (xx2 + zz2)) * scale3D.y;
	out._33 = (1 - (xx2 + yy2)) * scale3D.z;

	yz2 = rotation.y * z2;
	wx2 = rotation.w * x2;
	out._32 = (yz2 - wx2) * scale3D.z;
	out._23 = (yz2 + wx2) * scale3D.y;

	xy2 = rotation.x * y2;
	wz2 = rotation.w * z2;
	out._21 = (xy2 - wz2) * scale3D.y;
	out._12 = (xy2 + wz2) * scale3D.x;

	xz2 = rotation.x * z2;
	wy2 = rotation.w * y2;
	out._31 = (xz2 + wy2) * scale3D.z;
	out._13 = (xz2 - wy2) * scale3D.x;

	out._14 = 0;
	out._24 = 0;
	out._34 = 0;
	out._44 = 1;
}

// 世界坐标转屏幕坐标
bool Renderer::boneWorldToScreen(const Vector2 & screen_size, const Vector3 & pos, Vector2 & retPos, view_matrix_t matrix)
{
	float w = matrix[0][3] * pos.x + matrix[1][3] * pos.y + matrix[2][3] * pos.z + matrix[3][3];
	if (w < 0.001f)
		return false;

	float x = screen_size.x * .5f + (matrix[0][0] * pos.x + matrix[1][0] * pos.y + matrix[2][0] * pos.z + matrix[3][0]) / w * screen_size.x * .5f;
	float y = screen_size.y * .5f - (matrix[0][1] * pos.x + matrix[1][1] * pos.y + matrix[2][1] * pos.z + matrix[3][1]) / w * screen_size.y * .5f;

	retPos.x = x;
	retPos.y = y;

	return true;
}

// ------------------------------------------ImGui绘制------------------------------------------
// im绘制初始化
void Renderer::imDrawInit()
{
	if (!pImBuffer)
	{
		// 获得画笔对象
		//pImBuffer = ImGui::GetBackgroundDrawList();
		pImBuffer = ImGui::GetOverlayDrawList();
	}
}

// 画线
void Renderer::drawImLine(const Vector2 & p1, const Vector2 & p2, Color color, float thickness)
{
	imDrawInit();
	pImBuffer->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), GetU32(color), thickness);
}

// 画空心圆
void Renderer::drawImCircle(const Vector2 & center, float radius, int numSegments, Color color, float thickness)
{
	imDrawInit();
	pImBuffer->AddCircle(ImVec2(center.x, center.y), radius, GetU32(color), numSegments, thickness);
}

// 画实心圆
void Renderer::drawImCircleFilled(const Vector2 & center, float radius, int numSegments, Color color)
{
	imDrawInit();
	pImBuffer->AddCircleFilled(ImVec2(center.x, center.y), radius, GetU32(color), numSegments);
}

// 画文字
void Renderer::drawImText(const Vector2 & pos, const char * text, Color color, bool outline, float fontSize)
{
	imDrawInit();
	ImVec2 vec2 = ImVec2(pos.x, pos.y);
	ImVec2 textSize = Menu::get().pEspFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
	vec2.x -= textSize.x / 2.f;
	if (outline)
	{
		pImBuffer->AddText(Menu::get().pEspFont, fontSize, ImVec2(vec2.x + 1, vec2.y + 1), GetU32(Color(0, 0, 0)), text);
		pImBuffer->AddText(Menu::get().pEspFont, fontSize, ImVec2(vec2.x - 1, vec2.y - 1), GetU32(Color(0, 0, 0)), text);
		pImBuffer->AddText(Menu::get().pEspFont, fontSize, ImVec2(vec2.x + 1, vec2.y - 1), GetU32(Color(0, 0, 0)), text);
		pImBuffer->AddText(Menu::get().pEspFont, fontSize, ImVec2(vec2.x - 1, vec2.y + 1), GetU32(Color(0, 0, 0)), text);
	}
	pImBuffer->AddText(Menu::get().pEspFont, fontSize, vec2, GetU32(color), text);
}

// 画空心矩形
void Renderer::drawImRect(const Vector2 & pos, const Vector2 & size, Color color, float thickness)
{
	imDrawInit();
	pImBuffer->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), GetU32(color), 0, 0, thickness);
}

// 画实心矩形
void Renderer::drawImRectFilled(const Vector2 & pos, const Vector2 & size, Color color)
{
	imDrawInit();
	pImBuffer->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), GetU32(color), 0, 0);
}
