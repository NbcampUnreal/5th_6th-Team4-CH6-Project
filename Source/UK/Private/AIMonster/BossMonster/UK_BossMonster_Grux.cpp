#include "AIMonster/BossMonster/UK_BossMonster_Grux.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"

AUK_BossMonster_Grux::AUK_BossMonster_Grux()
{
	
	DetectionRadius = 2500.f;
	AttackRange = 350.f;
	AttackCooldown = 1.2f;
	MonsterType = EMonsterType::EliteGolem;
    
	CurrentPhase = 1;
}

void AUK_BossMonster_Grux::BeginPlay()
{
	Super::BeginPlay();
}

void AUK_BossMonster_Grux::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void AUK_BossMonster_Grux::ReceiveDamage(float Damage)
{
	Super::ReceiveDamage(Damage);
}

void AUK_BossMonster_Grux::UpdatePhase()
{
	if (!AttributeSet) return;

	float HPRatio = AttributeSet->GetHealth() / AttributeSet->GetMaxHealth();
	
	if (AttributeSet)
	{
		float CurHP = AttributeSet->GetHealth();
		float MaxHP = AttributeSet->GetMaxHealth();
		UE_LOG(LogTemp, Log, TEXT("Grux UpdatePhase Called! HP: %f / %f (Ratio: %f)"), CurHP, MaxHP, CurHP/MaxHP);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("UpdatePhase: AttributeSet is NULL!"));
		return;
	}
	
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
	float Distance = Target ? FVector::Dist(GetActorLocation(), Target->GetActorLocation()) : 0.f;
	
	if (CurrentPhase == 3)
	{
		AttackCooldown = 0.5f; 
		
		return Super::PlayRandomAttackMontage(); 
	}
	
	if (CurrentPhase == 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Distance: %f"), Distance)

		if (Distance > 280.f && JumpAttack)
		{
			UE_LOG(LogTemp, Error, TEXT("!!! DISTANCE OK - EXECUTING JUMP ATTACK !!!"));
			bIsAttacking = true; 
          
			PlayMontage(JumpAttack);
			
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AAIMonsterBase::OnAttackMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, JumpAttack);
			}
          
			return true;
		}
	}

	return Super::PlayRandomAttackMontage();
}

void AUK_BossMonster_Grux::ApplyBerserkBuff()
{
	UE_LOG(LogTemp, Error, TEXT("!!! Grux ENRAGED !!!"));
	
	AttackDamage *= 1.5f;
	
	GetCharacterMovement()->MaxWalkSpeed = 550.f;
	
	if (Berserk)
	{
		PlayMontage(Berserk);
	}
}
void AUK_BossMonster_Grux::ExecuteJumpSmashDamage()
{
	FVector Center = GetActorLocation();
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SmashRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// 디버그용 구체 (에디터에서 범위 확인용)
	DrawDebugSphere(GetWorld(), Center, SmashRadius, 32, FColor::Orange, false, 1.5f);

	if (GetWorld()->OverlapMultiByChannel(OverlapResults, Center, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (auto& Result : OverlapResults)
		{
			if (AActor* HitActor = Result.GetActor())
			{
				// 부모의 CalculateAoEDamage(플레이어 레벨 기반)를 활용하거나 수동 계산
				float Damage = CalculateAoEDamage(1); // 예시 레벨 1
				ApplyDamage(Damage, GetController());
			}
		}
	}
}
