#pragma once

#include "CoreMinimal.h"
#include "Character/EnemyCharacter.h"
#include "RiftBossCharacter.generated.h"

struct FOnAttributeChangeData;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(Blueprintable)
class RIFT_API ARiftBossCharacter : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	ARiftBossCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual int32 GetCurrentCombatPhase() const override;
	virtual bool IsCombatPhaseTransitioningForAI() const override;
	virtual void HandleAttributeDeath(AActor* DeathInstigator) override;
	virtual void HandleAttributeDamageNumber(float DamageAmount, bool bBlocked, const FVector& WorldLocation) override;

	UFUNCTION(BlueprintPure, Category="Boss|Phase")
	bool IsBossCombatPhaseTransitioning() const { return false; }

	UFUNCTION(BlueprintImplementableEvent, Category="Boss|Phase")
	void OnBossCombatPhaseChanged(int32 NewPhase);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phase")
	bool bEnablePhase2 = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float Phase2HealthRatio = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phase")
	TObjectPtr<UNiagaraSystem> Phase2EffectNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phase")
	FName Phase2EffectAttachSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phase")
	FVector Phase2EffectLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phase")
	FRotator Phase2EffectRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phase")
	FVector Phase2EffectScale = FVector::OneVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_CombatPhase, Category="Boss|Phase")
	int32 CurrentCombatPhase = 1;

private:
	UFUNCTION()
	void OnRep_CombatPhase();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartBossPhase2Effect();

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void CheckCombatPhaseFromHealth();
	void EnterCombatPhase(int32 NewPhase);
	void StartBossPhase2Effect();
	void StopBossPhase2Effect();

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActivePhase2EffectComponent;
};
