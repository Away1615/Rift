// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayTags/RiftGameplayTags.h"

#include "GameplayTagsManager.h"

FRiftGameplayTags FRiftGameplayTags::GameplayTags;

const FRiftGameplayTags& FRiftGameplayTags::Get()
{
	return GameplayTags;
}

void FRiftGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	GameplayTags.State_Movement_Grounded =
		Manager.AddNativeGameplayTag(TEXT("State.Movement.Grounded"));

	GameplayTags.State_Movement_Airborne =
		Manager.AddNativeGameplayTag(TEXT("State.Movement.Airborne"));

	GameplayTags.State_Movement_Climbing =
		Manager.AddNativeGameplayTag(TEXT("State.Movement.Climbing"));

	GameplayTags.State_Movement_Blocked =
		Manager.AddNativeGameplayTag(TEXT("State.Movement.Blocked"));

	GameplayTags.State_Action_Sprinting =
		Manager.AddNativeGameplayTag(TEXT("State.Action.Sprinting"));

	GameplayTags.State_Action_CoreActive =
		Manager.AddNativeGameplayTag(TEXT("State.Action.CoreActive"));

	GameplayTags.State_Core_PerfectWindow =
		Manager.AddNativeGameplayTag(TEXT("State.Core.PerfectWindow"));

	GameplayTags.State_Core_PerfectSuccess =
		Manager.AddNativeGameplayTag(TEXT("State.Core.PerfectSuccess"));

	GameplayTags.State_Weapon_Equipped =
		Manager.AddNativeGameplayTag(TEXT("State.Weapon.Equipped"));

	GameplayTags.State_Weapon_Equipping =
		Manager.AddNativeGameplayTag(TEXT("State.Weapon.Equipping"));

	GameplayTags.State_Weapon_Unequipping =
		Manager.AddNativeGameplayTag(TEXT("State.Weapon.Unequipping"));
}
