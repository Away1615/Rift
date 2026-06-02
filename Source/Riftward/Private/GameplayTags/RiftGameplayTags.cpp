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

	GameplayTags.Event_Ability_TwinSword_Core_DodgeFinished =
		Manager.AddNativeGameplayTag(TEXT("Event.Ability.TwinSword.Core.DodgeFinished"));

	GameplayTags.Event_Ability_TwinSword_Primary_ComboWindow =
		Manager.AddNativeGameplayTag(TEXT("Event.Ability.TwinSword.Primary.ComboWindow"));

	GameplayTags.Event_Ability_TwinSword_Primary_HitCheck =
		Manager.AddNativeGameplayTag(TEXT("Event.Ability.TwinSword.Primary.HitCheck"));

	GameplayTags.State_Ability_TwinSword_Core_PerfectDodgeWindow =
		Manager.AddNativeGameplayTag(TEXT("State.Ability.TwinSword.Core.PerfectDodgeWindow"));

}
