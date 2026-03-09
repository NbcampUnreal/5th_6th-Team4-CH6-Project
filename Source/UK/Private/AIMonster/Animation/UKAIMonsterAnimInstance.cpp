#include "AIMonster/Animation/UKAIMonsterAnimInstance.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
void UUKAIMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerMonster = Cast<AAIMonsterBase>(TryGetPawnOwner());
}
#pragma endregion

#pragma region Animation Update
void UUKAIMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if (!OwnerMonster) return;

    const FVector Velocity      = OwnerMonster->GetVelocity();
    const FVector ForwardVector = OwnerMonster->GetActorForwardVector();
    const FVector RightVector   = OwnerMonster->GetActorRightVector();

    Speed         = Velocity.Size2D();
    bIsDead       = OwnerMonster->IsDead();
    bIsAttacking  = OwnerMonster->bIsAttacking;
    bIsAggressive = OwnerMonster->GetIsAggressive();
    bIsChasing    = bIsAggressive && Speed > 10.f;

    // ── ForwardSpeed / RightSpeed ──────────────────────────────────────
    ForwardSpeed = FVector::DotProduct(ForwardVector, Velocity);
    RightSpeed   = FVector::DotProduct(RightVector,   Velocity);

    // ── Direction (BS Angle축, -180 ~ 180) ────────────────────────────
    if (!Velocity.IsNearlyZero(1.f))
    {
        const FVector VelNorm    = Velocity.GetSafeNormal2D();
        const float   ForwardDot = FVector::DotProduct(ForwardVector, VelNorm);
        const float   RightDot   = FVector::DotProduct(RightVector,   VelNorm);
        const float   RawAngle   = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
        Direction = FMath::FInterpTo(Direction, RawAngle, DeltaSeconds, 10.f);
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.f, DeltaSeconds, 8.f);
    }

    // ── LeanAngle (회전 속도 기반 좌우 기울기) ────────────────────────
    const float CurrentYaw = OwnerMonster->GetActorRotation().Yaw;
    const float YawDelta   = FMath::FindDeltaAngleDegrees(PrevYaw, CurrentYaw);
    // DeltaSeconds 0 guard
    const float YawSpeed   = (DeltaSeconds > KINDA_SMALL_NUMBER)
                             ? YawDelta / DeltaSeconds
                             : 0.f;
    const float TargetLean = FMath::Clamp(YawSpeed * 0.03f, -15.f, 15.f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 6.f);

    // ── TurnAngle (턴 애니메이션용 좌우 회전 각도) ────────────────────
    // YawDelta가 크면 회전 중, 작으면 정면
    const float TargetTurn = FMath::Clamp(YawDelta * 2.f, -90.f, 90.f);
    TurnAngle = FMath::FInterpTo(TurnAngle, TargetTurn, DeltaSeconds, 8.f);
    
    PrevYaw   = CurrentYaw;
}
#pragma endregion