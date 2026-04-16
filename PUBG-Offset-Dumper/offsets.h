#pragma once

#include <stdint.h>
#include <array>
#include "MemoryManager.h"

class offset {
public:
	uint64_t XenuineDecrypt;
	uint64_t UWorld;
	uint64_t GNames;
	uint64_t GNamesPtr = 0x10;
	uint64_t ChunkSize;
	uint64_t GObjects;

	uint64_t CurrentLevel;
	uint64_t Actors;
	uint64_t GameInstance;
	uint64_t LocalPlayer;

	uint64_t PlayerController;
	uint64_t AcknowledgedPawn;

	uint64_t PlayerCameraManager;
	uint64_t CameraCacheLocation;
	uint64_t CameraCacheRotation;
	uint64_t CameraCacheFOV;

	uint64_t RootComponent;
	uint64_t Mesh;
	uint64_t StaticMesh;
	uint64_t ComponentToWorld;
	uint64_t ComponentLocation;

	uint64_t LastTeamNum;
	uint64_t CharacterName;
	uint64_t SpectatedCount;

	uint64_t WeaponProcessor;
	uint64_t EquippedWeapons ;
	uint64_t CurrentWeaponIndex;
	uint64_t WeaponTrajectoryData;
	uint64_t TrajectoryConfig;

	uint64_t AnimScriptInstance;
	uint64_t ControlRotation_CP;
	uint64_t RecoilADSRotation_CP;
	uint64_t LeanLeftAlpha_CP;
	uint64_t LeanRightAlpha_CP;

	uint64_t LastSubmitTime;
	uint64_t LastRenderTimeOnScreen;
	uint64_t bAlwaysCreatePhysicsState;

	uint64_t HeaFlag;
	uint64_t Health1;
	uint64_t Health2;
	uint64_t Health3;
	uint64_t Health4;
	uint64_t Health5;
	uint64_t Health6;
	uint64_t Health_keys0;
	uint64_t Health_keys1;
	uint64_t Health_keys2;
	uint64_t Health_keys3;
	uint64_t Health_keys4;
	uint64_t Health_keys5;
	uint64_t Health_keys6;
	uint64_t Health_keys7;
	uint64_t Health_keys8;
	uint64_t Health_keys9;
	uint64_t Health_keys10;
	uint64_t Health_keys11;
	uint64_t Health_keys12;
	uint64_t Health_keys13;
	uint64_t Health_keys14;
	uint64_t Health_keys15;
	uint64_t GroggyHealth;

	uint64_t ObjID;
	uint64_t DecryptNameIndexRor;
	uint64_t DecryptNameIndexXorKey1;
	uint64_t DecryptNameIndexXorKey2;
	uint64_t DecryptNameIndexXorKey3;
	uint64_t DecryptNameIndexRval;
	uint64_t DecryptNameIndexSval;
	uint64_t DecryptNameIndexDval;
};

inline std::unique_ptr<offset>offsets;