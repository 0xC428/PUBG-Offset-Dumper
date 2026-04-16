#pragma once
#include <Windows.h>
#include <iostream>
#include <stdint.h>
#include <array>
#include <format>
#include "offsets.h"
#include <fstream>
#include <ctime>
#pragma warning(disable : 4996)
int main()
{
	time_t timer = time(NULL);
	printf("/* %d-%d-%d */\n", localtime(&timer)->tm_year + 1900, localtime(&timer)->tm_mon + 1, localtime(&timer)->tm_mday);
	printf("\n");

	memory = std::make_unique<Memory>();
	offsets = std::make_unique<offset>();
	std::string gameVersion = memory->GetProcessFileVersion(memory->proc_handle);
	printf("// v%s\n", gameVersion.c_str());

	unsigned char* dump;
	int imagesize = memory->module_size;
	// printf("size = %p\n", imagesize);
	dump = (unsigned char*)malloc(imagesize);
	memory->read(memory->module_base, dump, imagesize);
	
	/* XenuineDecrypt */
	auto decryptadd = memory->FindLastPattern(dump, imagesize, "48 8B 05 ? ? ? ? FF D0 48 8B D0");
	offsets->XenuineDecrypt = (unsigned int)(decryptadd + *(unsigned int*)(dump + decryptadd + 3) + 7);
	printf("constexpr uint64_t XenuineDecrypt = 0x%08X;\n", offsets->XenuineDecrypt);

	/* UWorld */
	auto uworldoffset = memory->FindPattern(dump, imagesize, "48 83 EC 28 E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? B0") + 0xA;
	uworldoffset = uworldoffset + memory->FindPattern(dump + uworldoffset, 1000, "e8 ? ? ? ?");
	uworldoffset = (unsigned int)(uworldoffset + *(unsigned long*)(dump + uworldoffset + 1) + 5);
	auto uworldaddr = uworldoffset + memory->FindPattern(dump + uworldoffset, 1000, "48 8B 15");
	offsets->UWorld = (unsigned int)(uworldaddr + *(unsigned int*)(dump + uworldaddr + 3) + 7);
	printf("constexpr uint64_t UWorld = 0x%08X;\n", offsets->UWorld);

	/* GNames */
	auto gname = memory->FindPattern(dump, imagesize, "39 05 ? ? ? ? 7f ? 0f 28 de");
	gname = gname + memory->FindPattern(dump + gname, 1000, "8D 0D") - 1;
	offsets->GNames = (unsigned int)(gname + *(unsigned int*)(dump + gname + 3) + 7) + 0x20;
	printf("constexpr uint64_t GNames = 0x%08X;\n", offsets->GNames);
	printf("constexpr uint64_t GNamesPtr = 0x%02X;\n", offsets->GNamesPtr);

	/* chunksize */
	unsigned long long chunksizeprefix = memory->FindPattern(dump, imagesize, "C1 ? ? 8B ? C1 ? 1F 03 ? 69 ? ? ? ? ? 44 2B F8");
	auto g_offset_chunksize = *(unsigned int*)(dump + chunksizeprefix + 12);
	if (g_offset_chunksize >= 0x1500)
	{
		offsets->ChunkSize = g_offset_chunksize;
	}
	else
	{
		chunksizeprefix = memory->FindPattern(dump, imagesize, "C1 ? ? 8B ? C1 ? 1F 03 ? 69 ? ? ? ? ? 44 2B F0");
		g_offset_chunksize = *(unsigned int*)(dump + chunksizeprefix + 12);
		offsets->ChunkSize = g_offset_chunksize;
	}
	printf("constexpr uint64_t ChunkSize = 0x%04X;\n", offsets->ChunkSize);

	/* GObjects */
	auto gobjectadd = memory->FindPattern(dump, imagesize, "48 8b c4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 56 48 83 ec ? 0f 29 70 ? 48 ?");
	if (gobjectadd + memory->FindPattern(dump + gobjectadd, 5000, "8B 15") - 1 != 0) {
		gobjectadd = gobjectadd + memory->FindPattern(dump + gobjectadd, 5000, "8B 15") - 1;
		offsets->GObjects = (unsigned int)(gobjectadd + *(unsigned int*)(dump + gobjectadd + 3) + 7);
	}
	else {
		gobjectadd = gobjectadd + memory->FindPattern(dump + gobjectadd, 5000, "8B 0D") - 1;
		offsets->GObjects = (unsigned int)(gobjectadd + *(unsigned int*)(dump + gobjectadd + 3) + 7);
	}
	printf("constexpr uint64_t GObjects = 0x%04X;\n", offsets->GObjects);
	

	printf("\n");


	/* CurrentLevel */
	auto currentlevel = memory->FindPattern(dump, imagesize, "48 85 C9 0F 84 ? ? ? ? 41 B0 01 48 8B D1 E8 ? ? ? ? 48 8B F0 48 85 C0 0F 84 ? ? ? ? 83 64 24 ? ? 83 64 24 ? ? E8 ? ? ? ? 48 83 3D ? ? ? ? ? 48 8B D8 48 8B 8E ? ? ? ? 75 14 4C 8B 05 ? ? ? ? 48 8B D1 B9 ? ? ? ? 41 FF D0 EB");
	offsets->CurrentLevel = *(unsigned int*)(dump + currentlevel + 0x3A + 0x3);
	printf("constexpr uint64_t CurrentLevel = 0x%04X;\n", offsets->CurrentLevel);

	/* actors */
	auto actors = memory->FindPattern(dump, imagesize, "e8 ? ? ? ? 44 8b c0 48 8d ? ? ? ? ? ? ? ? e8 ? ? ? ? b0");
	actors = (unsigned int)(actors + *(unsigned int*)(dump + actors + 1) + 5);
	offsets->Actors = *(unsigned char*)(dump + actors + 0x39 + 0x3);
	printf("constexpr uint64_t Actors = 0x%02X;\n", offsets->Actors);

	/* GameInstance */
	auto gameinstance = memory->FindPattern(dump, imagesize, "48 8B 8B ? ? ? ? 4D 85 C0 75 ? 48 8b 05");
	offsets->GameInstance = *(unsigned int*)(dump + gameinstance + 3);
	printf("constexpr uint64_t GameInstance = 0x%04X;\n", offsets->GameInstance);

	/* LocalPlayer */
	auto local = memory->FindPattern(dump, imagesize, "48 89 5C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 33 DB 48 8B F2 48 85 D2");
	offsets->LocalPlayer = *(unsigned int*)(dump + local + 0x25 + 0x3);
	printf("constexpr uint64_t LocalPlayer = 0x%02X;\n", offsets->LocalPlayer);


	printf("\n");


	/* PlayerController */
	auto playercontroller = memory->FindPattern(dump, imagesize, "40 53 48 83 EC 20 48 8B 01 FF 90 30010000 48 83");
	offsets->PlayerController = *(unsigned char*)(dump + playercontroller + 0x1E + 0x4);
	printf("constexpr uint64_t PlayerController = 0x%02X;\n", offsets->PlayerController);

	/* AcknowledgedPawn */
	auto acknowledgedpawn = memory->FindPattern(dump, imagesize, "48 8B C4 53 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 44 0F 29 40 ? 44 0F 29 48 ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 4D 8B E0 4C 89 44 24 ? 0F 28 F1 48 8B F9 ");
	offsets->AcknowledgedPawn = *(unsigned int*)(dump + acknowledgedpawn + 0x159 + 0x3);
	printf("constexpr uint64_t AcknowledgedPawn = 0x%01X;\n", offsets->AcknowledgedPawn);


	printf("\n");


	/* PlayerCameraManager */
	auto playercameramanager = memory->FindPattern(dump, imagesize, "48 8B 8B ? ? ? ? 48 ? ? ? ? ? 0F ? ? ? ? ? ? 0F ? ? ? ? ? ? ? ? F2 0F 10 81 ? ? ? ? F2 0F 11 07 8B 81 ? ? ? ? 89 47 08 F2 0F 10 81");
	offsets->PlayerCameraManager = *(unsigned int*)(dump + playercameramanager + 0x3);
	printf("constexpr uint64_t PlayerCameraManager = 0x%01X;\n", offsets->PlayerCameraManager);

	/* Camera */
	auto cam = memory->FindPattern(dump, imagesize, "48 8B 8B ? ? ? ? 48 ? ? ? ? ? 0F ? ? ? ? ? ? 0F ? ? ? ? ? ? ? ? F2 0F 10 81 ? ? ? ? F2 0F 11 07 8B 81 ? ? ? ? 89 47 08 F2 0F 10 81");
	offsets->CameraCacheLocation = *(unsigned int*)(dump + cam + 0x1D + 0x4);
	printf("constexpr uint64_t CameraCacheLocation = 0x%01X;\n", offsets->CameraCacheLocation);

	offsets->CameraCacheRotation = *(unsigned int*)(dump + cam + 0x32 + 0x4);
	printf("constexpr uint64_t CameraCacheRotation = 0x%01X;\n", offsets->CameraCacheRotation);

	auto FOV = memory->FindPattern(dump, imagesize, "F3 0F 10 81 ? ? ? ? 0F 2F 05 ? ? ? ? 77 08 F3 0F 10 81");
	offsets->CameraCacheFOV = *(unsigned int*)(dump + FOV + 0x11 + 0x4);
	printf("constexpr uint64_t CameraCacheFOV = 0x%01X;\n", offsets->CameraCacheFOV);


	printf("\n");


	auto rootcomponent = memory->FindPattern(dump, imagesize, "89 44 24 30 89 54 24 34 48 8B 44 24 ? 48 85 C0 0F 84 ? ? ? ? 48 8B ? ? ? ? ? ? 85 ? 75 ? 48 8B 05 ? ? ? ? 48 8B D1 B9 ? ? ? ? FF D0 ? 8B", 2);
	offsets->RootComponent = *(unsigned int*)(dump + rootcomponent + 0x16 + 0x3);
	printf("constexpr uint64_t RootComponent = 0x%01X;\n", offsets->RootComponent);

	auto mesh = memory->FindPattern(dump, imagesize, "48 8B 99 ? ? ? ? 48 85 DB 74 09 F6 83 ? ? ? ? ? 75 08");
	offsets->Mesh = *(unsigned int*)(dump + mesh + 0x3);
	printf("constexpr uint64_t Mesh = 0x%01X;\n", offsets->Mesh);

	auto smesh = memory->FindPattern(dump, imagesize, "48 03 94 CE ");
	offsets->StaticMesh = *(unsigned int*)(dump + smesh + 0x4);
	printf("constexpr uint64_t StaticMesh = 0x%01X;\n", offsets->StaticMesh);

	/* Component */
	auto world = memory->FindPattern(dump, imagesize, "0F 10 80 ? ? ? ? 0F 11 43 20 0F 10 88 ? ? ? ? 0F 11 4B 30 0F 10 80 ? ? ? ? 0F 11 43 40 48 8B 8B");
	offsets->ComponentToWorld = *(unsigned int*)(dump + world + 0x3);
	printf("constexpr uint64_t ComponentToWorld = 0x%01X;\n", offsets->ComponentToWorld);

	auto clocation = memory->FindPattern(dump, imagesize, "0F 10 88 ? ? ? ? 48 8D 44 24 ? 0F 28 C1 F3 0F 11 4C 24 ? 0F C6 C1 ? 0F C6 C9 ? F3 0F 11 4C 24 ? F3 0F 11 44 24");
	offsets->ComponentLocation = *(unsigned int*)(dump + clocation + 0x3);
	printf("constexpr uint64_t ComponentLocation = 0x%01X;\n", offsets->ComponentLocation);

	/*auto cvel = memory->FindPattern(dump, imagesize, "F2 0F 11 82 ? ? ? ? 8B 81 ? ? ? ? 89 82");
	offsets->ComponentVelocity = *(unsigned int*)(dump + cvel + 0x3);
	printf("constexpr uint64_t ComponentVelocity = 0x%01X;\n", offsets->ComponentVelocity);*/


	printf("\n");


	auto team = memory->FindPattern(dump, imagesize, "0F 85 ? ? ? ? 48 8B ? 48 8B ? FF 90 ? ? ? ? 41 3B 86");
	offsets->LastTeamNum = *(unsigned int*)(dump + team + 0x12 + 0x3);
	printf("constexpr uint64_t LastTeamNum = 0x%01X;\n", offsets->LastTeamNum);

	auto charactername = memory->FindLastPattern(dump, imagesize, "48 8D ? ? ? ? ? 48 8D 8C 24 ? ? ? ? E8 ? ? ? ? 48 8D ? ? ? ? ? 48 8D 8C 24 ? ? ? ? E8 ? ? ? ? 90");
	offsets->CharacterName = *(unsigned int*)(dump + charactername + 0x14 + 0x3);
	printf("constexpr uint64_t CharacterName = 0x%01X;\n", offsets->CharacterName);

	auto spectate = memory->FindPattern(dump, imagesize, "48 8B 03 48 8B CB FF 83 ? ? ? ? F3 0F 11 B3 ? ? ? ? FF 90 ? ? ? ? 48 85 C0 74 0D 83 B8 ");
	offsets->SpectatedCount = *(unsigned int*)(dump + spectate + 0x1F + 0x2);
	printf("constexpr uint64_t SpectatedCount = 0x%01X;\n", offsets->SpectatedCount);


	printf("\n");


	auto w = memory->FindPattern(dump, imagesize, "48 8B 8B ? ? ? ? E8 ? ? ? ? 84 C0 0F 85");
	offsets->WeaponProcessor = *(unsigned int*)(dump + w + 0x3);
	printf("constexpr uint64_t WeaponProcessor = 0x%01X;\n", offsets->WeaponProcessor);

	auto e = memory->FindPattern(dump, imagesize, "48 8B ? ? ? ? ? 48 8B 0C ? 48 8B 11 FF 92 ? ? ? ? 84 C0");
	offsets->EquippedWeapons = *(unsigned int*)(dump + e + 0x3);
	printf("constexpr uint64_t EquippedWeapons = 0x%01X;\n", offsets->EquippedWeapons);

	auto cw = memory->FindPattern(dump, imagesize, "44 0F B6 C2 84 D2 74 0E 41 83 F8 01 75 10 0F BE 81 ? ? ? ? C3");
	offsets->CurrentWeaponIndex = *(unsigned int*)(dump + cw + 0xE + 0x3);
	printf("constexpr uint64_t CurrentWeaponIndex = 0x%01X;\n", offsets->CurrentWeaponIndex);

	auto wt = memory->FindPattern(dump, imagesize, "75 ? 48 8B ? ? ? ? ? EB ? 48 8B ? ? ? ? ? 48 8B 5C 24 ? 48 83 C4 ? 5F C3");
	offsets->WeaponTrajectoryData = *(unsigned int*)(dump + wt + 0xB + 0x3);
	printf("constexpr uint64_t WeaponTrajectoryData = 0x%01X;\n", offsets->WeaponTrajectoryData);

	auto tc = memory->FindLastPattern(dump, imagesize, "F7 43 ? ? ? ? ? 75 ? 48 8B ? E8 ? ? ? ? F3 0F 10 80 ? ? ? ? EB");
	offsets->TrajectoryConfig = *(unsigned int*)(dump + tc + 0x11 + 0x4);
	printf("constexpr uint64_t TrajectoryConfig = 0x%01X;\n", offsets->TrajectoryConfig);


	printf("\n");


	auto as = memory->FindPattern(dump, imagesize, "48 8B 81 ? ? ? ? 48 85 C0 74 ? 80 78 ? ? 75 ? 48 8B 88 ? ? ? ? 48 85 C9 0F 85 ? ? ? ? ? C0 C3");
	offsets->AnimScriptInstance = *(unsigned int*)(dump + as + 0x3);
	printf("constexpr uint64_t AnimScriptInstance = 0x%01X;\n", offsets->AnimScriptInstance);

	auto cr = memory->FindPattern(dump, imagesize, "F2 0F ? ? ? ? ? ? 44 ? ? ? ? ? ? ? ? ? ? ? ? ? ? 44 ? ? ? ? ? ? 44 ? ? ? ? ? ? 44 ? ? ? ? ? ? ? 44 ? ? ? ? ? ? ? ? ? 44");
	offsets->ControlRotation_CP = *(unsigned int*)(dump + cr + 0x4);
	printf("constexpr uint64_t ControlRotation_CP = 0x%01X;\n", offsets->ControlRotation_CP);

	auto r = memory->FindPattern(dump, imagesize, "F3 0F 10 8F ? ? ? ? 48 8B 8F ? ? ? ? F3 0F 10");
	offsets->RecoilADSRotation_CP = *(unsigned int*)(dump + r + 0x4);
	printf("constexpr uint64_t RecoilADSRotation_CP = 0x%01X;\n", offsets->RecoilADSRotation_CP);

	auto ll = memory->FindPattern(dump, imagesize, "F3 0F 10 83 ? ? ? ? 41 0F 28 D8 41 0F 28 D5 E8 ? ? ? ? 8A 8B ? ? ? ? 44 0F 28 F8 F3 44 0F 11 BB ? ? ? ? 84 C9");
	offsets->LeanLeftAlpha_CP = *(unsigned int*)(dump + ll + 0x4);
	printf("constexpr uint64_t LeanLeftAlpha_CP = 0x%01X;\n", offsets->LeanLeftAlpha_CP);

	auto lr = memory->FindPattern(dump, imagesize, "F3 0F 10 83 ? ? ? ? 41 0F 28 D8 41 0F 28 D5 E8 ? ? ? ? 44 0F 28 C8 45 0F 28 D3 F3 44 0F 11 8B ? ? ? ? 44 0F 28 C7 84 C0");
	offsets->LeanRightAlpha_CP = *(unsigned int*)(dump + lr + 0x4);
	printf("constexpr uint64_t LeanRightAlpha_CP = 0x%01X;\n", offsets->LeanRightAlpha_CP);


	printf("\n");


	auto lst = memory->FindPattern(dump, imagesize, "F3 0F 10 9B ? ? ? ? 0F 28 CA F3 0F 5C CB F3 0F 10 05 ? ? ? ? 0F 2F C1");
	offsets->LastSubmitTime = *(unsigned int*)(dump + lst + 0x4);
	printf("constexpr uint64_t LastSubmitTime = 0x%01X;\n", offsets->LastSubmitTime);

	auto lrt = memory->FindPattern(dump, imagesize, "0F 2F 83 ? ? ? ? 41 0F 43 CE 83 E0 FB 0B C8 48 8B 03");
	offsets->LastRenderTimeOnScreen = *(unsigned int*)(dump + lrt + 0x3) + 0x4;
	printf("constexpr uint64_t LastRenderTimeOnScreen = 0x%01X;\n", offsets->LastRenderTimeOnScreen);

	auto acp = memory->FindPattern(dump, imagesize, "83 8F ? ? ? ? ? 48 8B 5C 24 ? 48 8B 74 24 ? 48 83 C4 20 5F C3");
	offsets->bAlwaysCreatePhysicsState = *(unsigned int*)(dump + acp + 0x2);
	printf("constexpr uint64_t bAlwaysCreatePhysicsState = 0x%01X;\n", offsets->bAlwaysCreatePhysicsState);


	printf("\n");


	auto health = memory->FindPattern(dump, imagesize, "80 ? ? ? ? ? ? 74 ? ? ? ? ? ? ? ? 74 ? ? ? ? ? ? ? ? 0F ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 74 ? 44 8B ? ? ? ? ? 48 8D ? ? ? E8");
	offsets->HeaFlag = *(unsigned int*)(dump + health + 0x2);
	offsets->Health1 = *(unsigned int*)(dump + health + 0x9 + 0x2);
	offsets->Health2 = *(unsigned int*)(dump + health + 0x4A + 0x4);
	offsets->Health3 = *(unsigned int*)(dump + health + 0x19 + 0x3);
	offsets->Health4 = *(unsigned int*)(dump + health + 0x20 + 0x5);
	offsets->Health5 = *(unsigned int*)(dump + health + 0x12 + 0x2);
	offsets->Health6 = *(unsigned int*)(dump + health + 0x31 + 0x3);
	printf("constexpr uint64_t HeaFlag = 0x%01X;\n", offsets->HeaFlag);
	printf("constexpr uint64_t Health1 = 0x%01X;\n", offsets->Health1);
	printf("constexpr uint64_t Health2 = 0x%01X;\n", offsets->Health2);
	printf("constexpr uint64_t Health3 = 0x%01X;\n", offsets->Health3);
	printf("constexpr uint64_t Health4 = 0x%01X;\n", offsets->Health4);
	printf("constexpr uint64_t Health5 = 0x%01X;\n", offsets->Health5);
	printf("constexpr uint64_t Health6 = 0x%01X;\n", offsets->Health6);

	auto Decrypthealth = (unsigned int)(health + 0x3D + 1 + *(unsigned int*)(dump + health + 0x3D + 1) + 4);
	offsets->Health_keys0 = *(unsigned int*)(dump + Decrypthealth + 0x17 + 0x3);
	offsets->Health_keys1 = *(unsigned int*)(dump + Decrypthealth + 0x20 + 0x3);
	offsets->Health_keys2 = *(unsigned int*)(dump + Decrypthealth + 0x27 + 0x3);
	offsets->Health_keys3 = *(unsigned int*)(dump + Decrypthealth + 0x2E + 0x3);
	offsets->Health_keys4 = *(unsigned int*)(dump + Decrypthealth + 0x35 + 0x3);
	offsets->Health_keys5 = *(unsigned int*)(dump + Decrypthealth + 0x3C + 0x3);
	offsets->Health_keys6 = *(unsigned int*)(dump + Decrypthealth + 0x43 + 0x3);
	offsets->Health_keys7 = *(unsigned int*)(dump + Decrypthealth + 0x4A + 0x3);
	offsets->Health_keys8 = *(unsigned int*)(dump + Decrypthealth + 0x51 + 0x3);
	offsets->Health_keys9 = *(unsigned int*)(dump + Decrypthealth + 0x58 + 0x3);
	offsets->Health_keys10 = *(unsigned int*)(dump + Decrypthealth + 0x5F + 0x3);
	offsets->Health_keys11 = *(unsigned int*)(dump + Decrypthealth + 0x66 + 0x3);
	offsets->Health_keys12 = *(unsigned int*)(dump + Decrypthealth + 0x6D + 0x3);
	offsets->Health_keys13 = *(unsigned int*)(dump + Decrypthealth + 0x74 + 0x3);
	offsets->Health_keys14 = *(unsigned int*)(dump + Decrypthealth + 0x7B + 0x3);
	offsets->Health_keys15 = *(unsigned int*)(dump + Decrypthealth + 0x82 + 0x3);
	printf("constexpr uint64_t Health_keys0 = 0x%01X;\n", offsets->Health_keys0);
	printf("constexpr uint64_t Health_keys1 = 0x%01X;\n", offsets->Health_keys1);
	printf("constexpr uint64_t Health_keys2 = 0x%01X;\n", offsets->Health_keys2);
	printf("constexpr uint64_t Health_keys3 = 0x%01X;\n", offsets->Health_keys3);
	printf("constexpr uint64_t Health_keys4 = 0x%01X;\n", offsets->Health_keys4);
	printf("constexpr uint64_t Health_keys5 = 0x%01X;\n", offsets->Health_keys5);
	printf("constexpr uint64_t Health_keys6 = 0x%01X;\n", offsets->Health_keys6);
	printf("constexpr uint64_t Health_keys7 = 0x%01X;\n", offsets->Health_keys7);
	printf("constexpr uint64_t Health_keys8 = 0x%01X;\n", offsets->Health_keys8);
	printf("constexpr uint64_t Health_keys9 = 0x%01X;\n", offsets->Health_keys9);
	printf("constexpr uint64_t Health_keys10 = 0x%01X;\n", offsets->Health_keys10);
	printf("constexpr uint64_t Health_keys11 = 0x%01X;\n", offsets->Health_keys11);
	printf("constexpr uint64_t Health_keys12 = 0x%01X;\n", offsets->Health_keys12);
	printf("constexpr uint64_t Health_keys13 = 0x%01X;\n", offsets->Health_keys13);
	printf("constexpr uint64_t Health_keys14 = 0x%01X;\n", offsets->Health_keys14);
	printf("constexpr uint64_t Health_keys15 = 0x%01X;\n", offsets->Health_keys15);


	auto ghealth = memory->FindPattern(dump, imagesize, "F3 0F 10 8B ? ? ? ? F3 0F 10 1D ? ? ? ? F3 0F");
	offsets->GroggyHealth = *(unsigned int*)(dump + ghealth + 0x4);
	printf("constexpr uint64_t GroggyHealth = 0x%01X;\n", offsets->GroggyHealth);


	printf("\n");


	/* ObjID,  DecryptName */
	auto decrypt = memory->FindPattern(dump, imagesize, "8B ? ? 8B ? ? 81 F2 ? ? ? ? 8B ? 8B");
	acknowledgedpawn += 0x1E;

	offsets->ObjID = *(unsigned char*)(dump + decrypt + 5);
	printf("constexpr uint64_t ObjID = 0x%02X;\n", offsets->ObjID);

	offsets->DecryptNameIndexRor = 0x1;
	printf("constexpr uint64_t DecryptNameIndexRor = 0x%01X;\n", offsets->DecryptNameIndexRor);

	offsets->DecryptNameIndexXorKey1 = *(unsigned int*)(dump + decrypt + 0x13 + 0x3);
	printf("constexpr uint64_t DecryptNameIndexXorKey1 = 0x%01X;\n", offsets->DecryptNameIndexXorKey1);
	offsets->DecryptNameIndexXorKey2 = *(unsigned int*)(dump + decrypt + 0x51 + 0x2);
	printf("constexpr uint64_t DecryptNameIndexXorKey2 = 0x%01X;\n", offsets->DecryptNameIndexXorKey2);
	offsets->DecryptNameIndexXorKey3 = *(unsigned int*)(dump + decrypt + 0x3D + 0x1);
	printf("constexpr uint64_t DecryptNameIndexXorKey3 = 0x%01X;\n", offsets->DecryptNameIndexXorKey3);

	offsets->DecryptNameIndexRval = *(unsigned char*)(dump + decrypt + 0x49 + 0x2);
	printf("constexpr uint64_t DecryptNameIndexRval = 0x%01X;\n", offsets->DecryptNameIndexRval);
	offsets->DecryptNameIndexSval = *(unsigned char*)(dump + decrypt + 0x36 + 0x2);
	printf("constexpr uint64_t DecryptNameIndexSval = 0x%01X;\n", offsets->DecryptNameIndexSval);
	offsets->DecryptNameIndexDval = *(unsigned char*)(dump + decrypt + 0x45 + 0x3);
	printf("constexpr uint64_t DecryptNameIndexDval = 0x%01X;\n", offsets->DecryptNameIndexDval);

	printf(
R"(
DWORD DecryptCIndex(DWORD value)
{
    return (((value ^ 0x%01X) << 0x%01X) | ((value ^ 0x%01X) >> 0x%01X) & 0x%01X) ^ __ROR4__(value ^ 0x%01X, 0x%01X) ^ 0x%01X;
}
)",
		offsets->DecryptNameIndexXorKey1,
		offsets->DecryptNameIndexDval,
		offsets->DecryptNameIndexXorKey1,
		offsets->DecryptNameIndexSval,
		offsets->DecryptNameIndexXorKey3,
		offsets->DecryptNameIndexXorKey1,
		offsets->DecryptNameIndexRval,
		offsets->DecryptNameIndexXorKey2);

	printf("\n");
	printf("\n");
	printf("\n");

	system("pause");
	return 1;
}