#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"

AUK_BossMonsterBase::AUK_BossMonsterBase()
{
	NetUpdateFrequency = 90.f;
	MinNetUpdateFrequency = 60.f;

	MonsterType = EMonsterType::EliteGolem;
}

void AUK_BossMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetPhase(EBossPhase::Phase1);
	}
}

void AUK_BossMonsterBase::ReceiveDamage(float Damage)
{
	Super::ReceiveDamage(Damage);

	if (HasAuthority())
	{
		UpdatePhase();
	}
}

void AUK_BossMonsterBase::UpdatePhase()
{
	if (!StatComponent) return;


	const float HPRatio = StatComponent->GetHP() / StatComponent->GetMaxHP();


	if (HPRatio <= EnrageHPRatio)
	{
		SetPhase(EBossPhase::Enrage);
	}
	else if (HPRatio <= Phase3HPRatio)
	{
		SetPhase(EBossPhase::Phase3);
	}
	else if (HPRatio <= Phase2HPRatio)
	{
		SetPhase(EBossPhase::Phase2);
	}
}

void AUK_BossMonsterBase::SetPhase(EBossPhase NewPhase)
{
	if (!HasAuthority()) return;
	if (CurrentPhase == NewPhase) return;

	CurrentPhase = NewPhase;

	OnRep_BossPhase();
}

void AUK_BossMonsterBase::OnRep_BossPhase()
{
	UE_LOG(LogTemp, Warning,
		TEXT("[Boss] Phase Changed : %d"),
		(int32)CurrentPhase);

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsEnum(TEXT("BossPhase"), ( uint8 )CurrentPhase);
		}
	}

	PlayRandomHitMontage();
}

bool AUK_BossMonsterBase::PlayRandomAttackMontage()
{
	return PlayPhasePattern();
}

bool AUK_BossMonsterBase::PlayPhasePattern()
{
	if (!HasAuthority()) return false;
	if (bIsHit || bIsAttacking || bIsDying) return false;


	TArray<UAnimMontage*>* PatternSet = nullptr;


	switch (CurrentPhase)
	{
	case EBossPhase::Phase1:
		PatternSet = &Phase1Patterns;
		break;

	case EBossPhase::Phase2:
		PatternSet = &Phase2Patterns;
		break;

	case EBossPhase::Phase3:
		PatternSet = &Phase3Patterns;
		break;

	case EBossPhase::Enrage:
		PatternSet = &EnragePatterns;
		break;
	}

	if (!PatternSet || PatternSet->Num() == 0)
		return false;


	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown)
		return false;


	const int32 Index =
		FMath::RandRange(0, PatternSet->Num() - 1);


	UAnimMontage* Montage = (*PatternSet)[Index];

	if (!Montage) return false;


	bIsAttacking = true;
	LastAttackTime = Now;

	Multicast_PlayAttackMontage(Index);

	return true;
}

void AUK_BossMonsterBase::ShowHPBar()
{
	Super::ShowHPBar();

	if (HPWidgetComponent)
	{
		HPWidgetComponent->SetDrawSize(FVector2D(400.f, 40.f));
	}
}

void AUK_BossMonsterBase::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUK_BossMonsterBase, CurrentPhase);
}
