// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/CustomCharacterMovementComponent.h"
#include "Character/UK_CharacterBase.h"

void UCustomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	switch ((ECustomMovementMode)CustomMovementMode)
	{
	case ECustomMovementMode::CMOVE_Glide:
		PhysGlide(deltaTime, Iterations);
		break;

	case ECustomMovementMode::CMOVE_Climb:
		PhysClimb(deltaTime, Iterations);
		break;
	default:
		Super::PhysCustom(deltaTime, Iterations);
		break;
	}
}

void UCustomCharacterMovementComponent::PhysGlide(float deltaTime, int32 Iterations)
{
	
	FVector Forward = CharacterOwner->GetActorForwardVector();

	Velocity += Forward * 500.f * deltaTime;
	Velocity.Z -= 300.f * deltaTime;

	MoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true);
}

void UCustomCharacterMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	FVector Input = ConsumeInputVector();

	FVector MoveDelta = Input * /*ClimbSpeed*/100 * deltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
}