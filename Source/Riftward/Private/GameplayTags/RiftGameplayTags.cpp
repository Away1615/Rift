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

	GameplayTags.Event_Ability_TwinSword_Secondary_ComboWindow =
		Manager.AddNativeGameplayTag(TEXT("Event.Ability.TwinSword.Secondary.ComboWindow"));

	GameplayTags.Event_Ability_TwinSword_Primary_HitCheck =
		Manager.AddNativeGameplayTag(TEXT("Event.Ability.TwinSword.Primary.HitCheck"));

	GameplayTags.State_Ability_TwinSword_Core_PerfectDodgeWindow =
		Manager.AddNativeGameplayTag(TEXT("State.Ability.TwinSword.Core.PerfectDodgeWindow"));

	GameplayTags.State_Ability_TwinSword_Core_PerfectDodgeEmpowered =
		Manager.AddNativeGameplayTag(TEXT("State.Ability.TwinSword.Core.PerfectDodgeEmpowered"));

	GameplayTags.State_Ability_TwinSword_Core_DodgeActive =
		Manager.AddNativeGameplayTag(TEXT("State.Ability.TwinSword.Core.DodgeActive"));

	GameplayTags.State_Ability_TwinSword_Primary_ComboActive =
		Manager.AddNativeGameplayTag(TEXT("State.Ability.TwinSword.Primary.ComboActive"));

	GameplayTags.State_Ability_TwinSword_Secondary_ComboActive =
	Manager.AddNativeGameplayTag(TEXT("State.Ability.TwinSword.Secondary.ComboActive"));

}
