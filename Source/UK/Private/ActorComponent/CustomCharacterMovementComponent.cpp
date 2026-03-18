// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/CustomCharacterMovementComponent.h"
#include "Character/UK_CharacterBase.h"

void UCustomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	switch ((ECustomMovementMode)CustomMovementMode)
	{
	case ECustomMovementMode::CMOVE_Climb:
		PhysClimb(deltaTime, Iterations);
		break;
	default:
		Super::PhysCustom(deltaTime, Iterations);
		break;
	}
}

void UCustomCharacterMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
		return;

	FVector Input = ConsumeInputVector();

	if (bIsClimbingSurface == false)
	{
		SetMovementMode(MOVE_Falling);
		return;
	}

	FVector WallNormal = CurrentClimbNormal;

	FVector Up = FVector::UpVector;
	FVector Right = FVector::CrossProduct(Up, WallNormal);
	FVector ClimbUp = FVector::CrossProduct(WallNormal, Right);

	FVector MoveDir =
		(ClimbUp * Input.X) +
		(Right * Input.Y);

	MoveDir = MoveDir.GetSafeNormal();

	float Speed = 200.f;
	FVector Delta = MoveDir * Speed * deltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	FVector Snap = -WallNormal * 50.f * deltaTime;
	SafeMoveUpdatedComponent(Snap, UpdatedComponent->GetComponentQuat(), true, Hit);

	FRotator TargetRot = (-WallNormal).Rotation();
	UpdatedComponent->SetWorldRotation(TargetRot);
}