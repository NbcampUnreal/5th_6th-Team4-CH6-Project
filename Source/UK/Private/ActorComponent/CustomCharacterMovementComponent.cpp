// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/CustomCharacterMovementComponent.h"
#include "Character/UK_CharacterBase.h"

void UCustomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	switch ((ECustomMovementMode)CustomMovementMode)
	{
	case ECustomMovementMode::CMOVE_Climb:
		break;
		
	case ECustomMovementMode::CMOVE_Gliding:
		PhysGlide(deltaTime, Iterations);
		break;
	default:
		Super::PhysCustom(deltaTime, Iterations);
		break;
	}
}

void UCustomCharacterMovementComponent::PhysGlide(float deltaTime, int32 Iterations)
{
	float SavedGravity = GravityScale;
	float SavedAirControl = AirControl;

	GravityScale = 0.f;
	AirControl = 0.8f;

	Velocity.Z = FMath::Max(Velocity.Z, -GlideFallSpeed);

	PhysFlying(deltaTime, Iterations);

	GravityScale = SavedGravity;
	AirControl = SavedAirControl;
}

bool UCustomCharacterMovementComponent::IsWalkable(const FHitResult& Hit) const
{
	if (const AUK_CharacterBase* Owner = Cast<AUK_CharacterBase>(GetOwner()))
	{
		if (Owner->IsFrying() && Hit.GetActor() && Hit.GetActor()->IsA<APawn>())
		{
			return false;
		}
	}
	return Super::IsWalkable(Hit);
}
