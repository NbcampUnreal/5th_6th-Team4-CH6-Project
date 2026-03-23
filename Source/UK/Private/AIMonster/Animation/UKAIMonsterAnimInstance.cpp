#include "AIMonster/Animation/UKAIMonsterAnimInstance.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/Monster/UK_BurrowMonster.h"

#pragma region Initialization
void UUKAIMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerMonster = Cast<AAIMonsterBase>(TryGetPawnOwner());
	OwnerBurrow  = Cast<AUK_BurrowMonster>(OwnerMonster);
}
#pragma endregion

#pragma region Animation Update
void UUKAIMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if (!OwnerMonster) return;

    const FVector Velocity = OwnerMonster->GetVelocity();
    Speed       = Velocity.Size2D();
    bIsDead     = OwnerMonster->IsDead();
    bIsAttacking  = OwnerMonster->bIsAttacking;
    bIsAggressive = OwnerMonster->GetIsAggressive();
    bIsChasing    = bIsAggressive && Speed > 10.f;
	bIsBurrowed   = OwnerBurrow ? OwnerBurrow->bIsBurrowed : false;

    if (Speed < 1.f)
    {
        Direction = FMath::FInterpTo(Direction, 0.f, DeltaSeconds, 8.f);
        LeanAngle = FMath::FInterpTo(LeanAngle, 0.f, DeltaSeconds, 6.f);
        TurnAngle = FMath::FInterpTo(TurnAngle, 0.f, DeltaSeconds, 8.f);

        const float CurrentYaw = OwnerMonster->GetActorRotation().Yaw;
        PrevYaw = CurrentYaw;
        ForwardSpeed = 0.f;
        RightSpeed   = 0.f;
        return;
    }

    const FVector ForwardVector = OwnerMonster->GetActorForwardVector();
    const FVector RightVector   = OwnerMonster->GetActorRightVector();

    ForwardSpeed = FVector::DotProduct(ForwardVector, Velocity);
    RightSpeed   = FVector::DotProduct(RightVector,   Velocity);

    // ── Direction ────────────────────────────────────────────────────────
    const FVector VelNorm    = Velocity.GetSafeNormal2D();
    const float   ForwardDot = FVector::DotProduct(ForwardVector, VelNorm);
    const float   RightDot   = FVector::DotProduct(RightVector,   VelNorm);
    const float   RawAngle   = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    Direction = FMath::FInterpTo(Direction, RawAngle, DeltaSeconds, 10.f);

    // ── LeanAngle ────────────────────────────────────────────────────────
    const float CurrentYaw = OwnerMonster->GetActorRotation().Yaw;
    const float YawDelta   = FMath::FindDeltaAngleDegrees(PrevYaw, CurrentYaw);
    const float YawSpeed   = (DeltaSeconds > KINDA_SMALL_NUMBER) ? YawDelta / DeltaSeconds : 0.f;
    const float TargetLean = FMath::Clamp(YawSpeed * 0.03f, -15.f, 15.f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 6.f);

    // ── TurnAngle ────────────────────────────────────────────────────────
    const float TargetTurn = FMath::Clamp(YawDelta * 2.f, -90.f, 90.f);
    TurnAngle = FMath::FInterpTo(TurnAngle, TargetTurn, DeltaSeconds, 8.f);

    PrevYaw = CurrentYaw;
}
#pragma endregion