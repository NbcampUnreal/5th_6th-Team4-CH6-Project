#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

class APawn;
class ACharacter;
class AAIController;
class AActor;
class UCharacterMovementComponent;

/**
 * BehaviorTree 태스크/서비스에서 공통으로 사용하는 유틸리티 함수들
 */
namespace UKBTHelpers
{
	// Movement Component 제어

	inline void SetOrientToMovement(APawn* Pawn, bool bEnable)
	{
		if (!Pawn) return;
		
		if (ACharacter* Char = Cast<ACharacter>(Pawn))
		{
			if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
			{
				MC->bOrientRotationToMovement = bEnable;
			}
		}
	}

	inline void StopMovementImmediately(APawn* Pawn)
	{
		if (!Pawn) return;
		
		if (ACharacter* Char = Cast<ACharacter>(Pawn))
		{
			if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
			{
				MC->StopMovementImmediately();
			}
		}
	}

	inline void RestoreRotationSettings(ACharacter* Char)
	{
		if (!Char) return;
		
		if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
		{
			MC->bOrientRotationToMovement     = true;
			MC->bUseControllerDesiredRotation = false;
		}
	}

	// 회전 제어

	inline void SnapFaceToTarget(APawn* Pawn, AActor* Target)
	{
		if (!Pawn || !Target) return;

		const FVector  ToTarget = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal();
		const FRotator FaceRot  = FRotationMatrix::MakeFromX(ToTarget).Rotator();
		Pawn->SetActorRotation(FRotator(0.f, FaceRot.Yaw, 0.f));
	}

	inline FRotator GetLookAtRotation(const FVector& From, const FVector& To)
	{
		const FVector Dir = (To - From).GetSafeNormal();
		const FRotator Rot = FRotationMatrix::MakeFromX(Dir).Rotator();
		return FRotator(0.f, Rot.Yaw, 0.f);
	}

	// AI Navigation

	inline bool RequestMoveTo(AAIController* AICon, const FVector& Dest, 
	                          float AcceptRadius = 50.f, bool bAllowPartial = false)
	{
		if (!AICon) return false;

		FAIMoveRequest MoveReq(Dest);
		MoveReq.SetAcceptanceRadius(AcceptRadius);
		MoveReq.SetUsePathfinding(true);
		MoveReq.SetAllowPartialPath(bAllowPartial);

		FNavPathSharedPtr NavPath;
		return AICon->MoveTo(MoveReq, &NavPath) != EPathFollowingRequestResult::Failed;
	}

	// 기타 유틸리티

	inline float Distance2D(const FVector& A, const FVector& B)
	{
		return FVector::Dist2D(A, B);
	}

	inline FVector GetFlatDirection(const FVector& Dir)
	{
		FVector Flat = Dir;
		Flat.Z = 0.f;
		return Flat.GetSafeNormal();
	}
}