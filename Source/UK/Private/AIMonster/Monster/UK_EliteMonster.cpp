#include "AIMonster/Monster/UK_EliteMonster.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

AUK_EliteMonster::AUK_EliteMonster()
{
	// 엘리트 기본값 — 블루프린트에서 덮어쓸 수 있음
	DetectionRadius  = 800.0f;
	PatrolRadius     = 1500.0f;
	MaxChaseDistance = 2500.0f;
	AttackDamage     = 30.0f;
	AttackRange      = 300.0f;
	AttackCooldown   = 1.8f;
	CorpseLingerTime = 8.0f;
}

//  특수 공격

bool AUK_EliteMonster::CanUseSpecialAttack() const
{
	if (!HasAuthority()) return false;
	if (bIsDying || bIsAttacking) return false;
	if (SpecialAttackMontages.Num() == 0) return false;

	float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	return (Now - LastSpecialAttackTime) >= SpecialAttackCooldown;
}

bool AUK_EliteMonster::PlaySpecialAttack()
{
	if (!CanUseSpecialAttack()) return false;

	int32 RandomIndex = FMath::RandRange(0, SpecialAttackMontages.Num() - 1);
	UAnimMontage* Montage = SpecialAttackMontages[RandomIndex];
	if (!Montage) return false;

	bIsAttacking = true;
	LastSpecialAttackTime = GetWorld()->GetTimeSeconds();

	Multicast_PlaySpecialAttackMontage(RandomIndex);
	return true;
}

void AUK_EliteMonster::Multicast_PlaySpecialAttackMontage_Implementation(int32 MontageIndex)
{
	if (!SpecialAttackMontages.IsValidIndex(MontageIndex)) return;

	UAnimMontage* Montage = SpecialAttackMontages[MontageIndex];
	if (!Montage) return;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	float Length = AnimInstance->Montage_Play(Montage, 1.0f);

	// 종료 콜백은 서버에서만 바인딩 (BT Task 에 알림)
	if (HasAuthority() && Length > 0.f)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AUK_EliteMonster::OnSpecialAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
	
	if (bShowSpecialAttackDebug && GetWorld())
	{
		const float CapsuleHalfHeight = GetCapsuleComponent()
			? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 90.f;
		const FVector FootLocation = GetActorLocation() - FVector(0.f, 0.f, CapsuleHalfHeight);
		const FVector Forward = GetActorForwardVector();

		// 바닥 원형 — 특수 공격 범위 표시 (납작한 실린더)
		DrawDebugCylinder(
			GetWorld(),
			FootLocation,
			FootLocation + FVector(0.f, 0.f, 10.f),
			SpecialAttackDebugRadius,
			32,
			FColor::Orange,
			false,
			SpecialAttackDebugDuration,
			0,
			3.f
		);

		// 전방 트레이스 캡슐 — MeleeTrace 노티파이 범위 가시화
		const FVector TraceStart    = FootLocation + FVector(0.f, 0.f, SpecialAttackDebugTraceHeight);
		const FVector TraceEnd      = TraceStart + Forward * SpecialAttackDebugTraceLength;
		const FVector CapsuleCenter = (TraceStart + TraceEnd) * 0.5f;
		const float   HalfHeight    = FVector::Dist(TraceStart, TraceEnd) * 0.5f + SpecialAttackDebugTraceRadius;
		const FQuat   CapsuleRot    = FRotationMatrix::MakeFromZ(TraceEnd - TraceStart).ToQuat();

		DrawDebugCapsule(
			GetWorld(),
			CapsuleCenter,
			HalfHeight,
			SpecialAttackDebugTraceRadius,
			CapsuleRot,
			FColor::Red,
			false,
			SpecialAttackDebugDuration,
			0,
			3.f
		);
	}

	UE_LOG(LogTemp, Log, TEXT("[Elite] %s: Playing SpecialAttack montage[%d]"), *GetName(), MontageIndex);
}

void AUK_EliteMonster::OnSpecialAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
	OnSpecialAttackFinished.ExecuteIfBound(!bInterrupted);
}

//  웨이포인트 순찰

FVector AUK_EliteMonster::GetNextWaypointLocation()
{
	if (PatrolWaypoints.Num() == 0) return SpawnLocation;

	// null 슬롯 건너뛰며 다음 유효한 웨이포인트 반환
	for (int32 i = 0; i < PatrolWaypoints.Num(); ++i)
	{
		AActor* WP = PatrolWaypoints[CurrentWaypointIndex];
		CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();

		if (WP)
		{
			return WP->GetActorLocation();
		}
	}

	// 모든 슬롯이 null인 경우 폴백
	return SpawnLocation;
}

//  Replication
void AUK_EliteMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUK_EliteMonster, CurrentWaypointIndex);
}