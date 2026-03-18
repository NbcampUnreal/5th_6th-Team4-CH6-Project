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

	// 1. 입력 가져오기
	FVector Input = ConsumeInputVector();

	// 2. 벽 체크
	if (!bIsClimbingSurface)
	{
		SetMovementMode(MOVE_Falling);
		return;
	}

	// 3. 벽 기준 좌표계 생성
	FVector WallNormal = CurrentClimbNormal;

	FVector Up = FVector::UpVector;
	FVector Right = FVector::CrossProduct(Up, WallNormal);
	FVector ClimbUp = FVector::CrossProduct(WallNormal, Right);

	// 4. 입력 → 이동 방향 변환
	FVector MoveDir =
		(ClimbUp * Input.X) +
		(Right * Input.Y);

	MoveDir = MoveDir.GetSafeNormal();

	// 5. 이동 속도 적용
	float Speed = 200.f;
	FVector Delta = MoveDir * Speed * deltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	// 6. 벽에 붙이기 (핵심)
	FVector Snap = -WallNormal * 50.f * deltaTime;
	SafeMoveUpdatedComponent(Snap, UpdatedComponent->GetComponentQuat(), true, Hit);

	// 7. 캐릭터 회전
	FRotator TargetRot = (-WallNormal).Rotation();
	UpdatedComponent->SetWorldRotation(TargetRot);
}