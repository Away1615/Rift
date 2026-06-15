// Fill out your copyright notice in the Description page of Project Settings.

#include "Debug/RiftDebugCVars.h"

#include "HAL/IConsoleManager.h"

namespace RiftDebugCVars
{
	static TAutoConsoleVariable<int32> CVarDebugTargetAssist(
		TEXT("Rift.Debug.TargetAssist"),
		0,
		TEXT("Draw Rift target assist scoring debug. 0=off, 1=on."),
		ECVF_Default
	);

	static TAutoConsoleVariable<int32> CVarDebugCombat(
		TEXT("Rift.Debug.Combat"),
		0,
		TEXT("Draw Rift combat debug. 0=off, 1=on."),
		ECVF_Default
	);

	static TAutoConsoleVariable<int32> CVarEnemyMovement(
		TEXT("Rift.Enemy.Movement"),
		1,
		TEXT("Enemy AI movement. 0=stationary(debug), 1=chase player."),
		ECVF_Default
	);

	bool IsTargetAssistDebugEnabled()
	{
		return CVarDebugTargetAssist.GetValueOnGameThread() != 0;
	}

	bool IsCombatDebugEnabled()
	{
		return CVarDebugCombat.GetValueOnGameThread() != 0;
	}

	bool IsEnemyMovementEnabled()
	{
		return CVarEnemyMovement.GetValueOnGameThread() != 0;
	}
}
