#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Action.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystem/GameplayEffects/GE_InstantDamage.h"
#include "Character/EnemyCharacter.h"
#include "Data/Player/Ability/PlayerAbilitySetConfig.h"
#include "DrawDebugHelpers.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_Action::UGA_TwinSword_Action()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_TwinSword_Action::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntry();
	if (!AbilityEntry)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	switch (AbilityEntry->AbilityType)
	{
	case ERiftAbilityType::TwinSwordSignature:
		ActivateSignature(Handle, ActorInfo, ActivationInfo);
		break;
	case ERiftAbilityType::TwinSwordEnhance:
		ActivateEnhance(Handle, ActorInfo, ActivationInfo);
		break;
	default:
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		break;
	}
}

void UGA_TwinSword_Action::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (bAddedBuffTag)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(ActiveBuffTag);
		}
	}

	ActiveBuffTag = FGameplayTag();
	bAddedBuffTag = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_TwinSword_Action::ActivateSignature(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	ApplyForwardDamage();

	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntry();
	const URiftAbilityMontageFragment* MontageFragment = AbilityEntry
		? AbilityEntry->FindFragment<URiftAbilityMontageFragment>()
		: nullptr;
	if (MontageFragment && MontageFragment->Montage)
	{
		PlayConfiguredMontage(true);
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_TwinSword_Action::ActivateEnhance(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntry();
	if (!AbilityEntry)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const URiftAbilityBuffFragment* BuffFragment = AbilityEntry->FindFragment<URiftAbilityBuffFragment>();
	if (!BuffFragment)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveBuffTag = BuffFragment->ActiveStateTag.IsValid()
		? BuffFragment->ActiveStateTag
		: FRiftGameplayTags::Get().State_Ability_TwinSword_Enhance_Active;

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->AddLooseGameplayTag(ActiveBuffTag);
		bAddedBuffTag = true;
	}

	PlayConfiguredMontage(false);

	if (BuffFragment->Duration <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilityTask_WaitDelay* BuffDurationTask =
		UAbilityTask_WaitDelay::WaitDelay(this, BuffFragment->Duration);
	BuffDurationTask->OnFinish.AddDynamic(this, &UGA_TwinSword_Action::HandleBuffFinished);
	BuffDurationTask->ReadyForActivation();
}

void UGA_TwinSword_Action::PlayConfiguredMontage(bool bEndOnMontageFinished)
{
	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntry();
	const URiftAbilityMontageFragment* MontageFragment = AbilityEntry
		? AbilityEntry->FindFragment<URiftAbilityMontageFragment>()
		: nullptr;
	if (!MontageFragment || !MontageFragment->Montage) return;

	const FName StartSection = MontageFragment->StartSection;
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageFragment->Montage,
			MontageFragment->PlayRate,
			StartSection,
			true
		);

	if (bEndOnMontageFinished)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGA_TwinSword_Action::HandleMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_TwinSword_Action::HandleMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_TwinSword_Action::HandleMontageInterrupted);
	}

	MontageTask->ReadyForActivation();
}

void UGA_TwinSword_Action::ApplyForwardDamage() const
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntry();
	const URiftAbilityHitFragment* HitFragment = AbilityEntry
		? AbilityEntry->FindFragment<URiftAbilityHitFragment>()
		: nullptr;
	AActor* SourceActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!AbilityEntry || !HitFragment || !SourceActor || !World || HitFragment->Damage <= 0.0f) return;

	const FVector Forward = SourceActor->GetActorForwardVector();
	const FVector Start = SourceActor->GetActorLocation() + Forward * 50.0f;
	const FVector End = Start + Forward * FMath::Max(0.0f, HitFragment->Range);
	const float Radius = FMath::Max(1.0f, HitFragment->Radius);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TwinSwordSignatureTrace), false, SourceActor);
	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(Radius);
	const FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);

	TArray<FHitResult> Hits;
	World->SweepMultiByObjectType(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		TraceShape,
		QueryParams
	);

	if (HitFragment->bDrawDebug)
	{
		DrawDebugLine(World, Start, End, FColor::Cyan, false, 0.5f, 0, 2.0f);
		DrawDebugSphere(World, End, Radius, 16, FColor::Cyan, false, 0.5f);
	}

	TSet<TWeakObjectPtr<AActor>> DamagedActors;
	for (const FHitResult& Hit : Hits)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Hit.GetActor());
		if (!Enemy) continue;

		const TWeakObjectPtr<AActor> DamagedActor(Enemy);
		if (DamagedActors.Contains(DamagedActor)) continue;

		DamagedActors.Add(DamagedActor);
		ApplyDamageToTarget(Enemy, Hit);
	}
}

void UGA_TwinSword_Action::ApplyDamageToTarget(AActor* TargetActor, const FHitResult& Hit) const
{
	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntry();
	const URiftAbilityHitFragment* HitFragment = AbilityEntry
		? AbilityEntry->FindFragment<URiftAbilityHitFragment>()
		: nullptr;
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!AbilityEntry || !HitFragment || !SourceASC || !TargetActor) return;

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(TargetActor);
	if (!Enemy) return;

	UAbilitySystemComponent* TargetASC = Enemy->GetAbilitySystemComponent();
	if (!TargetASC) return;

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddHitResult(Hit);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		UGE_InstantDamage::StaticClass(),
		GetAbilityLevel(),
		EffectContext
	);

	if (!SpecHandle.IsValid()) return;

	SpecHandle.Data->SetSetByCallerMagnitude(
		FRiftGameplayTags::Get().Data_Damage,
		-HitFragment->Damage
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(),
		TargetASC
	);
}

void UGA_TwinSword_Action::HandleBuffFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_TwinSword_Action::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_TwinSword_Action::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_TwinSword_Action::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
