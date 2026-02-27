#include "AIMonster/Monster/UK_PeacefulMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIMonster/UK_AiMonsterCtl.h"

#pragma region Initialization
AUK_PeacefulMonster::AUK_PeacefulMonster()
{
	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate              = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed              = 200.0f;

	// AI Controller
	AIControllerClass = AUK_AiMonsterCtl::StaticClass();
	AutoPossessAI     = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 평화 몬스터 설정
	Personality         = EMonsterPersonality::Peaceful;
	AlertDistance       = 300.0f;
	AllyCallRadius      = 1200.0f;
	ResetDistance       = 2500.0f;
	DetectionRadius     = 1000.0f;
	MaxChaseDistance    = 2000.0f;
	AttackDamage        = 15.0f;
	AttackRange         = 200.0f;
}

void AUK_PeacefulMonster::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		RequestState(EMonsterState::Passive);
	}
}
#pragma endregion