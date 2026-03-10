#include "AIMonster/BossMonster/UK_BossMonster_Grux.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "AIController.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Character/UK_CharacterBase.h"

AUK_BossMonster_Grux::AUK_BossMonster_Grux()
{
	
	DetectionRadius = 2500.f;
	AttackRange = 350.f;
	AttackCooldown = 1.2f;
	MonsterType = EMonsterType::Grux;
    
	CurrentPhase = 1;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.f, 130.f, 0.f);
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AUK_BossMonster_Grux::BeginPlay()
{
	Super::BeginPlay();
}

void AUK_BossMonster_Grux::UpdatePhase()
{
	if (!AttributeSet) return;

	float HPRatio = AttributeSet->GetHealth() / AttributeSet->GetMaxHealth();
	
	if (CurrentPhase == 1 && HPRatio <= 0.7f)
	{
		CurrentPhase = 2;
	}
	else if (CurrentPhase == 2 && HPRatio <= 0.3f)
	{
		CurrentPhase = 3;
		ApplyBerserkBuff();
	}
}

bool AUK_BossMonster_Grux::PlayRandomAttackMontage()
{
	if (bIsHit || bIsAttacking || bIsDying) return false;

	AActor* Target = GetTargetActor();
	AAIController* AICtl = Cast<AAIController>(GetController());
	if (Target)
	{
		GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &AUK_BossMonster_Grux::LookAtTargetSmooth, 0.01f, true);
	}
	
	if (AICtl && Target)
	{
		AICtl->SetFocus(Target, EAIFocusPriority::Gameplay);
	}
	
	float Distance = Target ? FVector::Dist(GetActorLocation(), Target->GetActorLocation()) : 0.f;
	int32 RandomValue = FMath::RandRange(0, 100);
	float CurrentPlayRate = (CurrentPhase == 3) ? BerserkPlayRate : 1.0f;
	
	if (CurrentPhase == 3)
	{
		AttackCooldown = 0.2f;
		if (Distance > 350.f && Distance < 1500.f)
		{
			if (RandomValue <= 50) return ExecuteJumpAttackAction(CurrentPlayRate);
			if (RandomValue <= 85) return ExecuteDashAttackAction(CurrentPlayRate);
		}
		return PlayBaseAttackWithSpeed(CurrentPlayRate);
	}
	
	if (CurrentPhase == 2)
	{
		if (Distance > 400.f && Distance < 1500.f)
		{
			if (RandomValue <= 10) return ExecuteJumpAttackAction(CurrentPlayRate);
			if (RandomValue > 10 && RandomValue <= 20) return ExecuteDashAttackAction(CurrentPlayRate);
			
			return false;
		}
	}
	
	if (Distance <= 350.f)
	{
		return PlayBaseAttackWithSpeed(CurrentPlayRate);
	}

	return false;
}

bool AUK_BossMonster_Grux::ExecuteJumpAttackAction(float PlayRate)
{
    if (!JumpAttack) return false;
    bIsAttacking = true;

    FVector LaunchDir = (GetTargetActor()->GetActorLocation() - GetActorLocation());
    LaunchDir.Z = 0.f;
    float PureDist = LaunchDir.Size();
    LaunchDir.Normalize();

	float MaxJumpDist = 900.f;
	float ActualJumpDist = FMath::Min(PureDist, MaxJumpDist);
	
	float JumpSpeed = (CurrentPhase == 3) ? 1100.f : 900.f;
	FVector HorizontalVelocity = LaunchDir * JumpSpeed;
	float UpStrength = (CurrentPhase == 3) ? 850.f : 900.f; 
	FVector FinalVelocity = HorizontalVelocity + FVector(0.f, 0.f, UpStrength);

	SetActorRotation(LaunchDir.Rotation());
	GetCharacterMovement()->StopMovementImmediately();
	
	LaunchCharacter(FinalVelocity, true, true);
    PlayMontage(JumpAttack, PlayRate);
    
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AAIMonsterBase::OnAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, JumpAttack);
    }
    return true;
}

bool AUK_BossMonster_Grux::ExecuteDashAttackAction(float PlayRate)
{
    if (!DashAttack) return false;
    bIsAttacking = true;

    DashDirection = (GetTargetActor()->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    SetActorRotation(DashDirection.Rotation());

    GetWorldTimerManager().SetTimer(DashTimerHandle, this, &AUK_BossMonster_Grux::ExecuteDashMove, 0.01f, true);
    
    float DashDuration = (CurrentPhase == 3) ? 0.4f : 0.7f;
    GetWorldTimerManager().SetTimer(StopDashTimerHandle, this, &AUK_BossMonster_Grux::StopDashMovement, DashDuration, false);

    PlayMontage(DashAttack, PlayRate);

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AUK_BossMonster_Grux::OnDashFinished);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, DashAttack);
    }
    return true;
}

void AUK_BossMonster_Grux::ApplyBerserkBuff()
{
	AttackDamage *= 1.5f;
	AttackCooldown = 0.4f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	BerserkPlayRate = 1.3f;
	
	if (Berserk)
	{
		PlayMontage(Berserk);
	}
}
void AUK_BossMonster_Grux::ExecuteJumpSmashDamage()
{
	FVector Center = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SmashRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	DrawDebugSphere(GetWorld(), Center, SmashRadius, 32, FColor::Orange, false, 2.f);
    
	if (GetWorld()->OverlapMultiByChannel(OverlapResults, Center, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (auto& Result : OverlapResults)
		{
			if (AActor* HitActor = Result.GetActor())
			{
				// 2. 플레이어인지 확인하고 ASC 가져오기
				if (AUK_CharacterBase* TargetPlayer = Cast<AUK_CharacterBase>(HitActor))
				{
					UAbilitySystemComponent* TargetASC = TargetPlayer->GetAbilitySystemComponent();
					if (TargetASC)
					{
						// 3. 무기 공격과 동일하게 어트리뷰트에 직접 데미지 적용
						// CalculateAoEDamage(1)의 리턴값이 AttackDamage 역할을 한다고 보시면 됩니다.
						float FinalDamage = CalculateAoEDamage(1);

						TargetASC->ApplyModToAttribute(
						   UUK_PlayerStatusAttributeSet::GetDamageAttribute(), 
						   EGameplayModOp::Additive, 
						   FinalDamage
						);

						// 확인용 디버그
						UE_LOG(LogTemp, Warning, TEXT("Smash Hit! Target: %s, Damage: %f"), *HitActor->GetName(), FinalDamage);
						DrawDebugString(GetWorld(), HitActor->GetActorLocation(), TEXT("SMASH HIT!"), nullptr, FColor::Red, 1.0f);
					}
				}
			}
		}
	}
}

void AUK_BossMonster_Grux::OnDashFinished(UAnimMontage* Montage, bool bInterrupted)
{
	StopDashMovement();
	GetWorldTimerManager().ClearTimer(StopDashTimerHandle);
	GetWorldTimerManager().ClearTimer(RotationTimerHandle);
	
	OnAttackMontageEnded(Montage, bInterrupted);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

}
void AUK_BossMonster_Grux::ExecuteDashMove()
{
	float SpeedPerFrame = (CurrentPhase == 3) ? 15.f : 10.f; 
	FHitResult Hit;
	AddActorWorldOffset(DashDirection * SpeedPerFrame, true, &Hit);

	if (Hit.bBlockingHit)
	{
		StopDashMovement();
	}
}

void AUK_BossMonster_Grux::StopDashMovement()
{
	GetWorldTimerManager().ClearTimer(DashTimerHandle);
}

void AUK_BossMonster_Grux::LookAtTargetSmooth()
{
	AActor* Target = GetTargetActor();
	if (!Target || bIsDying) 
	{
		GetWorldTimerManager().ClearTimer(RotationTimerHandle);
		return;
	}
	
	FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	FRotator TargetRot = Dir.Rotation();
	FRotator SmoothRot = FMath::RInterpTo(GetActorRotation(), TargetRot, 0.01f, 10.0f);
    
	SetActorRotation(SmoothRot);
}

bool AUK_BossMonster_Grux::PlayBaseAttackWithSpeed(float PlayRate)
{
	bool bSuccess = Super::PlayRandomAttackMontage();
    
	if (bSuccess)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
			if (CurrentMontage)
			{
				AnimInstance->Montage_SetPlayRate(CurrentMontage, PlayRate);
			}
		}
	}
	return bSuccess;
}