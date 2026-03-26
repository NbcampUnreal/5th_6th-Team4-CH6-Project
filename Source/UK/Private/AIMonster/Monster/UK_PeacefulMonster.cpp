#include "AIMonster/Monster/UK_PeacefulMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "BehaviorTree/BlackboardComponent.h"

#pragma region Initialization
AUK_PeacefulMonster::AUK_PeacefulMonster()
{
	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseRVOAvoidance = false;
	GetCharacterMovement()->RotationRate              = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed              = 200.0f;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

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
	
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
}

void AUK_PeacefulMonster::BeginPlay()
{
	Super::BeginPlay();

	RequestState(EMonsterState::Passive);
}

void AUK_PeacefulMonster::NotifyAttacked(AController* InstigatorController)
{
	Super::NotifyAttacked(InstigatorController);

	if (!bCounterAttackOnHit || bIsAggressive) return;
	if (!InstigatorController) return;

	AActor* Attacker = InstigatorController->GetPawn();
	if (!Attacker) return;

	bIsAggressive = true;
	Personality   = EMonsterPersonality::Aggressive;

	if (AUK_AiMonsterCtl* AICtl = Cast<AUK_AiMonsterCtl>(GetController()))
	{
		if (UBlackboardComponent* BB = AICtl->GetBlackboardComponent())
		{
			BB->SetValueAsObject(TEXT("TargetPlayer"), Attacker);
		}
		AICtl->SetCurrentTarget(Attacker);
	}

	RequestState(EMonsterState::Chase);
}
#pragma endregion
