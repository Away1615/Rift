#include "Tools/GameTools.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagContainer.h"

void GameTools::SendGameplayEvent(const USkeletalMeshComponent* MeshComp, const FGameplayTag& EventTag, float EventMagnitude)
{
	if (!MeshComp || !EventTag.IsValid()) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = Owner;
	EventData.Target = Owner;
	EventData.EventMagnitude = EventMagnitude;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, EventData);
}
