#include "AIMonster/BossMonster/UK_BossMonster_Grux.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "AIController.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Character/UK_CharacterBase.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AIMonster/BossMonster/UK_BossProjectileBase.h"
#include "Animation/AnimInstance.h"

AUK_BossMonster_Grux::AUK_BossMonster_Grux()
{
	MaxChaseDistance = 5000.f;
	DetectionRadius = 2500.f;
	AttackRange = 350.f;
	AttackCooldown = 4.5f;
	MonsterType = EMonsterType::Grux;
	SmashRadius = 600.f;
	CurrentPhase = 1;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.f, 130.f, 0.f);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	JumpTargetDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("JumpTargetDecal"));
	JumpTargetDecal->SetupAttachment(RootComponent);
	
	JumpTargetDecal->SetVisibility(false);
	JumpTargetDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	
	FVector DecalSize = FVector(50.f, JumpTargetRadius, JumpTargetRadius);
	JumpTargetDecal->DecalSize = DecalSize;
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
		SmashRadius = 600.f;
	}
	else if (CurrentPhase == 2 && HPRatio <= 0.3f)
	{
		CurrentPhase = 3;
		SmashRadius = 750.f;
		if (JumpTargetDecal)
		{
			JumpTargetDecal->DecalSize = FVector(50.f, SmashRadius, SmashRadius);
		}
		ApplyBerserkBuff();
	}
}

bool AUK_BossMonster_Grux::PlayRandomAttackMontage()
{
	if (bIsAttacking || bIsDying) return false;
	if (bIsHit) return false;
	
	AActor* Target = GetTargetActor();
	AAIController* AICtl = Cast<AAIController>(GetController());
	if (Target)
	{
		if (!GetWorldTimerManager().IsTimerActive(RotationTimerHandle))
		{
			GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &AUK_BossMonster_Grux::LookAtTargetSmooth, 0.05f, true);
		}
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
		AttackCooldown = 1.0f;
		if (Distance > 350.f && Distance < 1500.f)
		{
			if (RandomValue <= 20) return ExecuteRangedAttackAction(CurrentPlayRate);
			if (RandomValue <= 50) return ExecuteJumpAttackAction(CurrentPlayRate);
			if (RandomValue <= 85) return ExecuteDashAttackAction(CurrentPlayRate);
			
			return false;
		}
		
		if (Distance <= 350.f) 
			return PlayBaseAttackWithSpeed(CurrentPlayRate);
		
		return false;
	}
	
	if (CurrentPhase == 2)
	{
		if (Distance > 400.f && Distance < 1500.f)
		{
			if (RandomValue <= 10) return ExecuteRangedAttackAction(CurrentPlayRate);
			if (RandomValue > 10 && RandomValue <= 20) return ExecuteJumpAttackAction(CurrentPlayRate);
			if (RandomValue > 20 && RandomValue <= 25) return ExecuteDashAttackAction(CurrentPlayRate);
			
			return false;
		}
		
		if (Distance <= 350.f) 
				return PlayBaseAttackWithSpeed(CurrentPlayRate);
		
		return false;
	}
	
	if (Distance <= 350.f)
	{
		return PlayBaseAttackWithSpeed(CurrentPlayRate);
	}
	
	return false;
}

bool AUK_BossMonster_Grux::ExecuteRangedAttackAction(float PlayRate)
{
	CurrentAttackType = EMonsterAttackType::Normal;
	
	if (!RangedAttackMontage) return false;
	bIsAttacking = true;
	
	AActor* Target = GetTargetActor();
	if (Target)
	{
		FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		SetActorRotation(Dir.Rotation());
	}

	PlayMontage(RangedAttackMontage, PlayRate);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AAIMonsterBase::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, RangedAttackMontage);
	}

	return true;
}

bool AUK_BossMonster_Grux::ExecuteJumpAttackAction(float PlayRate)
{
	CurrentAttackType = EMonsterAttackType::Special;
	
    if (!JumpAttack) return false;
    bIsAttacking = true;
	
	FVector StartLoc = GetActorLocation();
	FVector TargetLoc = GetTargetActor()->GetActorLocation();
	
	if (JumpTargetDecal)
	{
		JumpTargetDecal->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		FVector DecalLocation = TargetLoc;
		DecalLocation.Z = StartLoc.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		JumpTargetDecal->SetWorldLocation(DecalLocation);
		JumpTargetDecal->SetVisibility(true);
	}
	
	GetCharacterMovement()->BrakingDecelerationFalling = 0.f;
	GetCharacterMovement()->AirControl = 0.f;
	
	FVector Dir = TargetLoc - StartLoc;
	float Dist = Dir.Size2D();
	
	float BonusHeight = (CurrentPhase == 3) ? 600.f : 500.f;
	float FlightTime = (CurrentPhase == 3) ? 1.2f : 1.4f;
	float OverShootCorrection = 0.75f;
	
	FVector HorizontalDir = Dir;
	HorizontalDir.Z = 0.f;
	HorizontalDir.Normalize();
	FVector HorizontalVel = HorizontalDir * (Dist / FlightTime) * OverShootCorrection;
	
	float Gravity = GetWorld()->GetGravityZ() * -1.f;
	float VerticalVel = (TargetLoc.Z - StartLoc.Z + (0.5f * Gravity * FMath::Square(FlightTime))) / FlightTime;
	VerticalVel += (BonusHeight / FlightTime);
	FVector FinalVelocity = HorizontalVel + FVector(0.f, 0.f, VerticalVel);
	
	SetActorRotation(HorizontalDir.Rotation());
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
	CurrentAttackType = EMonsterAttackType::Special;
	
    if (!DashAttack) return false;
    bIsAttacking = true;

    DashDirection = (GetTargetActor()->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    SetActorRotation(DashDirection.Rotation());

    GetWorldTimerManager().SetTimer(DashTimerHandle, this, &AUK_BossMonster_Grux::ExecuteDashMove, 0.033f, true);
    
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
	
	if (BerserkLoopEffect && !BerserkComponent)
	{
		BerserkComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			BerserkLoopEffect,
			GetMesh(),
			BerserkSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true 
		);

		if (BerserkComponent)
		{
			BerserkComponent->SetRelativeScale3D(FVector(1.5f));
		}
	}
	
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
				if (AUK_CharacterBase* TargetPlayer = Cast<AUK_CharacterBase>(HitActor))
				{
					UAbilitySystemComponent* TargetASC = TargetPlayer->GetAbilitySystemComponent();
					if (TargetASC)
					{
						float FinalDamage = CalculateAoEDamage(1);

						TargetASC->ApplyModToAttribute(
						   UUK_PlayerStatusAttributeSet::GetDamageAttribute(), 
						   EGameplayModOp::Additive, 
						   FinalDamage
						);
						DrawDebugString(GetWorld(), HitActor->GetActorLocation(), TEXT("SMASH HIT!"), nullptr, FColor::Red, 1.0f);
					}
				}
			}
		}
	}
	if (JumpTargetDecal)
	{
		JumpTargetDecal->SetVisibility(false);
		FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);
		JumpTargetDecal->AttachToComponent(GetRootComponent(), AttachRules);
		
		JumpTargetDecal->SetRelativeLocation(FVector::ZeroVector);
		JumpTargetDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
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
	float SpeedPerFrame = (CurrentPhase == 3) ? 50.f : 33.f; 
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
	FRotator SmoothRot = FMath::RInterpTo(GetActorRotation(), TargetRot, 0.033f, 10.0f);
    
	SetActorRotation(SmoothRot);
}

void AUK_BossMonster_Grux::ResetForReturn()
{
	Super::ResetForReturn();
    
	CurrentPhase = 1;
	AttackCooldown = 4.5f;
	AttackDamage /= (BerserkPlayRate > 1.0f ? 1.5f : 1.0f); // 버서크 대미지 원복
	BerserkPlayRate = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = 475.f;
    
	if (BerserkComponent)
	{
		BerserkComponent->DestroyComponent();
		BerserkComponent = nullptr;
	}
    
	GetWorldTimerManager().ClearTimer(DashTimerHandle);
	GetWorldTimerManager().ClearTimer(StopDashTimerHandle);
	GetWorldTimerManager().ClearTimer(RotationTimerHandle);
}

bool AUK_BossMonster_Grux::PlayBaseAttackWithSpeed(float PlayRate)
{
	CurrentAttackType = EMonsterAttackType::Normal;
	
	bool bSuccess = Super::PlayRandomAttackMontage();
    
	if (bSuccess)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
			if (CurrentMontage)
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AAIMonsterBase::OnAttackMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
				
				AnimInstance->Montage_SetPlayRate(CurrentMontage, PlayRate);
				bIsAttacking = true;
			}
		}
	}
	return bSuccess;
}

void AUK_BossMonster_Grux::LaunchSwordWave()
{
	if (!SwordWaveClass)
	{
		return;
	}
	FVector MuzzleLocation = GetMesh()->GetSocketLocation(MuzzleSocketName);
	FRotator SpawnRotation = GetActorRotation(); 

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	
	AUK_BossProjectileBase* Projectile = GetWorld()->SpawnActor<AUK_BossProjectileBase>(
	   SwordWaveClass, MuzzleLocation, SpawnRotation, SpawnParams
	);

	if (Projectile)
	{
		Projectile->DamageAmount = CalculateAoEDamage(1);
	}
}