#include "AbilitySystem/GameplayAbilities/GA_TwinSword_ConfigDrivenAction.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/GameplayEffects/GE_DurationTag.h"
#include "AbilitySystem/GameplayEffects/GE_InstantDamage.h"
#include "Character/EnemyCharacter.h"
#include "Data/Player/Ability/AbilityDefinitionConfig.h"
#include "Data/Player/Ability/Fragments/AbilityActionExecutionFragment.h"
#include "Data/Player/Ability/Fragments/AbilityTimedStateFragment.h"
#include "Data/Player/Ability/Fragments/AbilityEffectListFragment.h"
#include "Data/Player/Ability/Fragments/AbilityHitDetectionFragment.h"
#include "Data/Player/Ability/Fragments/AbilityMontageSectionsFragment.h"
#include "DrawDebugHelpers.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_ConfigDrivenAction::UGA_TwinSword_ConfigDrivenAction()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_TwinSword_ConfigDrivenAction::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinition();
	if (!AbilityDefinition)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UAbilityActionExecutionFragment* ExecFragment = AbilityDefinition->FindFragment<UAbilityActionExecutionFragment>();
	if (!ExecFragment)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PendingExecutionMode = ExecFragment->ExecutionMode;

	// 应用施法中状态 Tag（DataAsset Active State Tag 字段），用于阻止 Dodge 在释放期间打断本技能
	const FGameplayTag ActiveStateTag = GetAbilityActiveStateTag();
	if (ActiveStateTag.IsValid())
	{
		CastingStateHandle = ApplyInfiniteStateTagEffect(Handle, ActorInfo, ActivationInfo, ActiveStateTag);
	}

	// 如果配置了事件 Tag，等 Montage Notify 触发后再执行效果
	if (ExecFragment->ExecutionEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			ExecFragment->ExecutionEventTag,
			nullptr,
			true,  // OnlyMatchExact
			true   // OnlyTriggerOnce
		);
		EventTask->EventReceived.AddDynamic(this, &UGA_TwinSword_ConfigDrivenAction::HandleExecutionEvent);
		EventTask->ReadyForActivation();
	}

	// 先播 Montage，无论是否事件驱动
	if (!PlayActionMontage(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 无事件 Tag：激活时立即执行（兼容旧行为）
	if (!ExecFragment->ExecutionEventTag.IsValid())
	{
		ExecuteAction();
	}
}

bool UGA_TwinSword_ConfigDrivenAction::PlayActionMontage(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinition();
	const UAbilityMontageSectionsFragment* SectionsFragment = AbilityDefinition
		? AbilityDefinition->FindFragment<UAbilityMontageSectionsFragment>()
		: nullptr;

	if (!SectionsFragment || SectionsFragment->Sections.IsEmpty()) return false;

	const FAbilityMontageSection& Section = SectionsFragment->Sections[0];
	UAnimMontage* MontageToPlay = SectionsFragment->Montage
		? SectionsFragment->Montage.Get()
		: Section.Montage.Get();
	if (!MontageToPlay) return false;

	return TryPlayMontage(Handle, ActorInfo, MontageToPlay, Section.PlayRate, Section.SectionName, true);
}

void UGA_TwinSword_ConfigDrivenAction::HandleExecutionEvent(FGameplayEventData Payload)
{
	ExecuteAction();
}

void UGA_TwinSword_ConfigDrivenAction::ExecuteAction()
{
	switch (PendingExecutionMode)
	{
	case EAbilityActionExecutionMode::ForwardHit:
		ApplyForwardDamage();
		break;
	case EAbilityActionExecutionMode::ApplyTimedState:
		ApplyTimedStateEffect();
		break;
	}
}

bool UGA_TwinSword_ConfigDrivenAction::ApplyTimedStateEffect() const
{
	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinition();
	const UAbilityTimedStateFragment* TimedStateFragment = AbilityDefinition
		? AbilityDefinition->FindFragment<UAbilityTimedStateFragment>()
		: nullptr;
	if (!TimedStateFragment || TimedStateFragment->Duration <= 0.0f) return false;

	TSubclassOf<UGameplayEffect> StateEffectClass = TimedStateFragment->StateEffectClass;
	if (!StateEffectClass)
	{
		StateEffectClass = UGE_DurationTag::StaticClass();
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		StateEffectClass,
		GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)
	);
	if (!SpecHandle.IsValid()) return false;

	SpecHandle.Data->SetSetByCallerMagnitude(FRiftGameplayTags::Get().Data_Duration, TimedStateFragment->Duration);

	if (TimedStateFragment->ActiveStateTag.IsValid())
	{
		SpecHandle.Data->DynamicGrantedTags.AddTag(TimedStateFragment->ActiveStateTag);
	}

	ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
	return true;
}

void UGA_TwinSword_ConfigDrivenAction::ApplyForwardDamage() const
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinition();
	const UAbilityHitDetectionFragment* HitDetectionFragment = AbilityDefinition
		? AbilityDefinition->FindFragment<UAbilityHitDetectionFragment>()
		: nullptr;
	const UAbilityEffectListFragment* EffectListFragment = AbilityDefinition
		? AbilityDefinition->FindFragment<UAbilityEffectListFragment>()
		: nullptr;
	AActor* SourceActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!HitDetectionFragment || !EffectListFragment || !SourceActor || !World) return;

	float Damage = 0.0f;
	for (const FAbilityEffectConfig& Effect : EffectListFragment->Effects)
	{
		if (Effect.Type == EAbilityEffectKind::Damage && Effect.Magnitude > 0.0f)
		{
			Damage = Effect.Magnitude;
			break;
		}
	}
	if (Damage <= 0.0f) return;

	const FVector Forward = SourceActor->GetActorForwardVector();
	const FVector Start = SourceActor->GetActorLocation() + Forward * 50.0f;
	const FVector End = Start + Forward * FMath::Max(0.0f, HitDetectionFragment->Range);
	const float Radius = FMath::Max(1.0f, HitDetectionFragment->Radius);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TwinSwordConfigDrivenTrace), false, SourceActor);
	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(Radius);
	const FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);

	TArray<FHitResult> Hits;
	World->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity, ObjectQueryParams, TraceShape, QueryParams);

	if (HitDetectionFragment->bDrawDebug)
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
		ApplyDamageToTarget(Enemy, Hit, Damage);
	}
}

void UGA_TwinSword_ConfigDrivenAction::ApplyDamageToTarget(AActor* TargetActor, const FHitResult& Hit, const float Damage) const
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC || !TargetActor || Damage <= 0.0f) return;

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

	SpecHandle.Data->SetSetByCallerMagnitude(FRiftGameplayTags::Get().Data_Damage, -Damage);
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void UGA_TwinSword_ConfigDrivenAction::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	// 确保施法中状态 Tag 被移除（正常结束/中断/取消场景均覆盖）
	RemoveGrantedStateTagEffect(CastingStateHandle);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
