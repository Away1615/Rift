#include "Combat/RiftHitReactionTypes.h"

#include "GameFramework/Actor.h"

ERiftHitReactDirection CalculateHitReactDirection(const AActor* TargetActor, const FVector& InstigatorLocation)
{
	if (!TargetActor)
	{
		return ERiftHitReactDirection::Front;
	}

	FVector DirectionToInstigator = InstigatorLocation - TargetActor->GetActorLocation();
	DirectionToInstigator.Z = 0.0f;
	if (!DirectionToInstigator.Normalize())
	{
		return ERiftHitReactDirection::Front;
	}

	FVector Forward = TargetActor->GetActorForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();

	const float ForwardDot = FVector::DotProduct(Forward, DirectionToInstigator);
	return ForwardDot >= 0.0f ? ERiftHitReactDirection::Front : ERiftHitReactDirection::Back;
}

FName GetHitReactSectionName(const ERiftHitReactDirection Direction)
{
	switch (Direction)
	{
	case ERiftHitReactDirection::Front:
		return TEXT("Front");
	case ERiftHitReactDirection::Back:
		return TEXT("Back");
	default:
		return NAME_None;
	}
}
