#include "Renderer.h"
#include "Memory.h"
#include "Menu.h"
#include "Player.h"
#include "Config.h"

void Renderer::drawFrames()
{
	float minCrossCenter = 99999999.0f;
	Menu::get().drawMainMenu();
	view_matrix_t matrix = Memory::get().read<view_matrix_t>(GlobalVars::get().viewMatrixAddr);

	// 临时的最佳自瞄目标
	shared_ptr<Player> bestAimTarget = nullptr;

	/*if (GlobalVars::get().localPlayer != nullptr && transformCoord(GlobalVars::get().localPlayer, matrix))
	{
		baseAddrEsp(GlobalVars::get().localPlayer);
		lineEsp(GlobalVars::get().localPlayer);
	}*/

	//cout << GlobalVars::get().activeEnemyCounter << endl;
	for (int i = 0; i < GlobalVars::get().activeEnemyCounter; i++)
	{
		if (playerWorldToScreen(GlobalVars::get().enemyList[i], matrix))
		{
			//baseAddrEsp(GlobalVars::get().enemyList[i]);

			// 骨骼计算失败，过滤死人
			if (!caluMatchstickMen(GlobalVars::get().enemyList[i], matrix))
			{
				//cout << "计算失败" << endl;
				continue;
			}

			// 透视
			if (GlobalVars::get().enemyList[i]->distance <= Config::get().espRange)
			{
				// 方框透视
				if (Config::get().boxEsp)
				{
					boxEsp(GlobalVars::get().enemyList[i]);
				}

				// 连线透视
				if (Config::get().lineEsp)
				{
					lineEsp(GlobalVars::get().enemyList[i]);
				}

				// 血量透视
				if (Config::get().hpEsp)
				{
					hpEsp(GlobalVars::get().enemyList[i]);
				}

				// 距离透视
				if (Config::get().distanceEsp)
				{
					distanceEsp(GlobalVars::get().enemyList[i]);
				}

				// 骨骼透视
				if (Config::get().boneEsp)
				{
					if (GlobalVars::get().enemyList[i]->type == PlayerType::enemy)
					{
						drawMatchstickMen(GlobalVars::get().enemyList[i], matrix, Config::get().espColor);
					}
					else if (Config::get().openFriendEsp)
					{
						drawMatchstickMen(GlobalVars::get().enemyList[i], matrix, Color::Green);
					}
				}
			}

			// 自瞄
			if (Config::get().aimbot && (GlobalVars::get().enemyList[i]->distance > 5 && GlobalVars::get().enemyList[i]->distance <= Config::get().aimbotRange) && lockAimTarget == nullptr)
			{
				// 开了瞄准镜后，计算不准确了
				// 准星距离，目标距离准星的距离，取所有目标中距离准星最小的。还有一种筛选自瞄目标的方式是取所有目标距离自己最近的。
				/*float xDiff = GlobalVars::get().drawRect.centerX - GlobalVars::get().enemyList[i]->box.centerX;
				float yDiff = GlobalVars::get().drawRect.centerY - GlobalVars::get().enemyList[i]->box.centerY;
				float crossCenter = sqrt(pow(xDiff, 2) + pow(yDiff, 2));
				if (crossCenter < minCrossCenter)
				{
					minCrossCenter = crossCenter;
					bestAimTarget = GlobalVars::get().enemyList[i];
				}*/

				// 绘制目标离准星的距离，用于测试
				/*if (false)
				{
					char text[50];
					sprintf_s(text, "%.0f, %.0f, %.0f",
							  crossCenter,
							  GlobalVars::get().enemyList[i]->box.centerX,
							  GlobalVars::get().enemyList[i]->box.centerY);

					drawImText(Vector2(GlobalVars::get().enemyList[i]->box.x, GlobalVars::get().enemyList[i]->box.y + GlobalVars::get().enemyList[i]->box.height), text, Config::get().espColor);
				}*/

				if (GlobalVars::get().enemyList[i]->distance < minCrossCenter)
				{
					minCrossCenter = GlobalVars::get().enemyList[i]->distance;
					bestAimTarget = GlobalVars::get().enemyList[i];
				}
			}
		}
	}

	if (Config::get().aimbot && bestAimTarget != nullptr)
	{
		// 锁定自瞄目标
		lockAimTarget = bestAimTarget;
	}

	// 自瞄
	if (Config::get().aimbot && GetAsyncKeyState(VK_RBUTTON) & 0x8000)
	{
		if (lockAimTarget != nullptr)
		{
			// 防止切换瞄准镜也会自瞄
			aimCounter++;
			if (aimCounter > 15)
			{
				aimbot(lockAimTarget, matrix);
				/*if (lockAimTarget->hp < 1)
				{
					aimCounter = 0;
					lockAimTarget = nullptr;
				}
				else
				{
					aimbot(lockAimTarget, matrix);
				}*/
			}
		}
	}
	else
	{
		aimCounter = 0;
		lockAimTarget = nullptr;
	}

	// 无后座
	if (Config::get().noRecoil || Config::get().lockBullet)
	{
		noRecoil();
	}

	Menu::get().endMenuScene();
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
	// 怎么打印出自己身上的所有地址？
	//cout << text << endl;

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

	drawImText(Vector2(tmpBox.centerX, tmpBox.y), player->bpCName.c_str(), color, false, 25);
	//drawImText(Vector2(tmpBox.centerX, tmpBox.y), text, color, false, 25);
	drawImRect(Vector2(player->box.x, player->box.y), Vector2(player->box.width, player->box.height), color);
}

// 人物加速
void Renderer::increaseSpeed()
{
	Memory::get().write<float>(GlobalVars::get().localPlayer->base + GlobalVars::get().ofs.playerSpeed_offset, Config::get().moveSpeed);
}

// 子弹锁定和无后座力
void Renderer::noRecoil()
{
	uintptr_t bulletBaseAddr = Memory::get().read<uintptr_t>(GlobalVars::get().baseAddr + GlobalVars::get().ofs.bullet);
	uintptr_t offset1Addr = Memory::get().read<uintptr_t>(bulletBaseAddr + GlobalVars::get().ofs.bullet_offset1);
	uintptr_t offset2Addr = Memory::get().read<uintptr_t>(offset1Addr + GlobalVars::get().ofs.bullet_offset2);
	uintptr_t offset3Addr = Memory::get().read<uintptr_t>(offset2Addr + GlobalVars::get().ofs.bullet_offset3);

	uintptr_t gun1Addr = Memory::get().read<uintptr_t>(offset3Addr);
	uintptr_t gun2Addr = Memory::get().read<uintptr_t>(offset3Addr + 8);
	uintptr_t gun3Addr = Memory::get().read<uintptr_t>(offset3Addr + 16);

	// 子弹锁定
	if (Config::get().lockBullet)
	{
		Memory::get().write<int>(gun1Addr + GlobalVars::get().ofs.bullet_count_offset, 100);
		Memory::get().write<int>(gun2Addr + GlobalVars::get().ofs.bullet_count_offset, 100);
		Memory::get().write<int>(gun3Addr + GlobalVars::get().ofs.bullet_count_offset, 100);
	}

	// 无后坐力
	if (Config::get().noRecoil)
	{
		Memory::get().write<int>(gun1Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_recoil_x_offset, 0);
		Memory::get().write<int>(gun1Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_recoil_y_offset, 0);
		Memory::get().write<int>(gun1Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_jitter_offset, 0);

		Memory::get().write<int>(gun2Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_recoil_x_offset, 0);
		Memory::get().write<int>(gun2Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_recoil_y_offset, 0);
		Memory::get().write<int>(gun2Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_jitter_offset, 0);

		Memory::get().write<int>(gun3Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_recoil_x_offset, 0);
		Memory::get().write<int>(gun3Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_recoil_y_offset, 0);
		Memory::get().write<int>(gun3Addr + GlobalVars::get().ofs.bullet_count_offset + GlobalVars::get().ofs.gun_jitter_offset, 0);
	}
}

bool Renderer::playerWorldToScreen(shared_ptr<Player> player, view_matrix_t matrix)
{
	float w = matrix[0][3] * player->origin.x + matrix[1][3] * player->origin.y + matrix[2][3] * player->origin.z + matrix[3][3];
	if (w < 0.001f)
		return false;

	player->distance = w / 100.0f;

	/*float x1 = (pos.x + 0);
	float x2 = (pos.x + 200);
	float x3 = (pos.x + 200);

	float y1 = (pos.y + 0);
	float y2 = (pos.y + 200);
	float y3 = (pos.y + 200);

	float z1 = (pos.z + 0);
	float z2 = (pos.z + 110);
	float z3 = (pos.z - 110);

	float centerX = screen_size.x * .5f + (matrix[0][0] * x1 + matrix[1][0] * y1 + matrix[2][0] * z1 + matrix[3][0]) / w * screen_size.x * .5f;
	float minY = screen_size.y * .5f - (matrix[0][1] * x2 + matrix[1][1] * y2 + matrix[2][1] * z2 + matrix[3][1]) / w * screen_size.y * .5f;
	float maxY = screen_size.y * .5f - (matrix[0][1] * x3 + matrix[1][1] * y3 + matrix[2][1] * z3 + matrix[3][1]) / w * screen_size.y * .5f;*/

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

void Renderer::boxEsp(shared_ptr<Player> player)
{
	//float thikness = player->distance <= 1200.f ? thikness = 1.f : thikness = 0.5f;
	if (player->type == PlayerType::enemy)
	{
		drawImRect(Vector2(player->box.x, player->box.y), Vector2(player->box.width, player->box.height), Config::get().espColor);
	}
	else if (Config::get().openFriendEsp)
	{
		drawImRect(Vector2(player->box.x, player->box.y), Vector2(player->box.width, player->box.height), Color::Green);
	}
}

void Renderer::lineEsp(shared_ptr<Player> player)
{
	if (player->type == PlayerType::enemy)
	{
		drawImLine(Vector2(GlobalVars::get().drawRect.centerX, 0), Vector2(player->box.centerX, player->box.y), Config::get().espColor);
	}
	else if (Config::get().openFriendEsp)
	{
		drawImLine(Vector2(GlobalVars::get().drawRect.centerX, 0), Vector2(player->box.centerX, player->box.y), Color::Green);
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
		drawImText(Vector2(player->box.x, player->box.y + player->box.height), text, Config::get().espColor);
	}
	else if (Config::get().openFriendEsp)
	{
		drawImText(Vector2(player->box.x, player->box.y + player->box.height), text, Color::Green);
	}
}

void Renderer::aimbot(shared_ptr<Player> player, view_matrix_t matrix)
{
	// 重新读数据，这样延迟小，但消耗大
	view_matrix_t matrix1 = Memory::get().read<view_matrix_t>(GlobalVars::get().viewMatrixAddr);
	uintptr_t meshAddr = Memory::get().read<uintptr_t>(player->base + GlobalVars::get().ofs.playerMesh_offset);
	uintptr_t skeletonArrayAddr = Memory::get().read<uintptr_t>(meshAddr + GlobalVars::get().ofs.playerBoneArray_offset);
	uintptr_t skeletonMatrixAddr = meshAddr + GlobalVars::get().ofs.playerComponentToWorld_offset;

	//uintptr_t skeletonMatrixAddr = player->skeletonMatrixAddr;
	//uintptr_t skeletonArrayAddr = player->skeletonArrayAddr;

	Vector2 screenSize = Vector2(GlobalVars::get().drawRect.width, GlobalVars::get().drawRect.height);
	Vector2 bone2D;

	int index = 5;
	if (Config::get().aimbotType == 0)
	{
		index = 5;
	}
	else if (Config::get().aimbotType == 1)
	{
		index = 3;
	}

	if (aimbootWorldToScreen(screenSize, getBonePos(skeletonMatrixAddr, skeletonArrayAddr + index * 48), bone2D, matrix1))
	{
		//cout << "base=" << player->base << ",index=" << index << ",bone2D.x=" << bone2D.x << ",bone2D.y=" << bone2D.x << endl;
		mouse_event(MOUSEEVENTF_MOVE, bone2D.x, bone2D.y, 0, 0);
	}
}

// 世界坐标转屏幕坐标
bool Renderer::aimbootWorldToScreen(const Vector2 & screen_size, const Vector3 & pos, Vector2 & retPos, view_matrix_t matrix)
{
	float w = matrix[0][3] * pos.x + matrix[1][3] * pos.y + matrix[2][3] * pos.z + matrix[3][3];
	if (w < 0.001f)
		return false;

	float x = (matrix[0][0] * pos.x + matrix[1][0] * pos.y + matrix[2][0] * pos.z + matrix[3][0]) / w * screen_size.x * .5f;
	float y = -(matrix[0][1] * pos.x + matrix[1][1] * pos.y + matrix[2][1] * pos.z + matrix[3][1]) / w * screen_size.y * .5f;

	retPos.x = x;
	retPos.y = y;

	return true;
}

// 计算骨头人骨骼
bool Renderer::caluMatchstickMen(shared_ptr<Player> player, view_matrix_t matrix)
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
																return true;
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
	return false;
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
																drawImCircle(Vector2(boneData.head.x, boneData.head.y), player->box.width * 0.15f, player->box.width * 0.2f, color);
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
			drawImText(bone2D, text, Config::get().espColor);
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
