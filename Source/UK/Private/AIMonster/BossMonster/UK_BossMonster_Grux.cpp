#include "AIMonster/BossMonster/UK_BossMonster_Grux.h"

AUK_BossMonster_Grux::AUK_BossMonster_Grux()
{
	
	DetectionRadius = 2500.f;
	AttackRange = 350.f;
	AttackCooldown = 1.2f;

	MonsterType = EMonsterType::EliteGolem;
}

void AUK_BossMonster_Grux::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("Golem King Spawned"));
}

void AUK_BossMonster_Grux::OnPhaseChanged(EBossPhase NewPhase)
{
	switch (NewPhase)
	{
	case EBossPhase::Phase2:
		UE_LOG(LogTemp, Warning, TEXT("Phase2 Activated"));
		break;

	case EBossPhase::Enrage:
		UE_LOG(LogTemp, Warning, TEXT("ENRAGE MODE"));
		break;
	}
}