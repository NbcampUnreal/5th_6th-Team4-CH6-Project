#include "AIMonster/Monster/UK_EliteMonster.h"
#include "Character/UK_CharacterBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

#pragma region Initialization
AUK_EliteMonster::AUK_EliteMonster()
{
	DetectionRadius  = 800.0f;
	PatrolRadius     = 1500.0f;
	MaxChaseDistance = 2500.0f;
	AttackDamage     = 30.0f;
	AttackRange      = 300.0f;
	AttackCooldown   = 1.8f;
	CorpseLingerTime = 8.0f;
}

void AUK_EliteMonster::InitializeStatsFromPlayerLevel(int32 PlayerLevel)
{
	// 부모: AttackDamage + Defense 설정
	Super::InitializeStatsFromPlayerLevel(PlayerLevel);

	// 엘리트 전용: 광역 공격 데미지 = (PlayerLevel × 3.14) × 1.5
	SpecialAttackDamage = CalculateAoEDamage(PlayerLevel);
}
#pragma endregion

#pragma region Special Attack
bool AUK_EliteMonster::CanUseSpecialAttack() const
{
	if (bIsDying || bIsAttacking) return false;
	if (SpecialAttackMontages.Num() == 0) return false;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	return (Now - LastSpecialAttackTime) >= SpecialAttackCooldown;
}

bool AUK_EliteMonster::PlaySpecialAttack()
{
	if (!CanUseSpecialAttack()) return false;

	const int32 RandomIndex = FMath::RandRange(0, SpecialAttackMontages.Num() - 1);
	UAnimMontage* Montage   = SpecialAttackMontages[RandomIndex];
	if (!Montage) return false;

	bIsAttacking          = true;
	bIsSpecialAttacking   = true; 
	LastSpecialAttackTime = GetWorld()->GetTimeSeconds();

	// 몽타주 길이 미리 계산
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	const float MontageLength = AnimInst ? Montage->GetPlayLength() : 1.0f;

	// 임팩트 타이밍: 몽타주 길이의 40% 시점에 광역 데미지 (BP에서 SpecialAttackHitTiming으로 조절)
	const float HitDelay = MontageLength * SpecialAttackHitTiming;
	GetWorldTimerManager().SetTimer(
		SpecialAttackAoETimerHandle, this,
		&AUK_EliteMonster::ApplySpecialAttackAoE,
		HitDelay, false);

	PlaySpecialAttackMontage(RandomIndex);
	return true;
}

void AUK_EliteMonster::PlaySpecialAttackMontage(int32 MontageIndex)
{
	if (!SpecialAttackMontages.IsValidIndex(MontageIndex)) return;

	UAnimMontage* Montage = SpecialAttackMontages[MontageIndex];
	if (!Montage) return;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	const float Length = AnimInstance->Montage_Play(Montage, 1.0f);

	if (Length > 0.f)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AUK_EliteMonster::OnSpecialAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
}

void AUK_EliteMonster::ApplySpecialAttackAoE()
{
	if (!GetWorld()) return;

	const FVector Center = GetActorLocation();

	FCollisionObjectQueryParams ObjParams(FCollisionObjectQueryParams::AllObjects);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(
		Overlaps, Center, FQuat::Identity, ObjParams,
		FCollisionShape::MakeSphere(SpecialAttackAoERadius), QueryParams);

	TSet<AActor*> DamagedActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || DamagedActors.Contains(HitActor)) continue;

		AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(HitActor);
		if (!Player) continue;

		DamagedActors.Add(HitActor);
		//Player->ReceiveDamage(SpecialAttackDamage);
	}
}

void AUK_EliteMonster::OnSpecialAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 어보트 시 타이머 취소
	if (bInterrupted)
	{
		GetWorldTimerManager().ClearTimer(SpecialAttackAoETimerHandle);
	}

	bIsAttacking = false;
	bIsSpecialAttacking = false; 
	OnSpecialAttackFinished.ExecuteIfBound(); 
}
#pragma endregion