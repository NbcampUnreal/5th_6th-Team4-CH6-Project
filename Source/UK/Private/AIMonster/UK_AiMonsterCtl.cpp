#include "AIMonster/UK_AiMonsterCtl.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/UK_CharacterBase.h"

#pragma region Initialization
AUK_AiMonsterCtl::AUK_AiMonsterCtl()
{
	PrimaryActorTick.bCanEverTick = false;

	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius                              = 1200.f;
	SightConfig->LoseSightRadius                          = 1500.f;
	SightConfig->PeripheralVisionAngleDegrees             = 360.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}
#pragma endregion

#pragma region Possess / UnPossess
void AUK_AiMonsterCtl::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledMonster = Cast<AAIMonsterBase>(InPawn);
	if (!ControlledMonster) return;

	// 회전 설정
	ControlledMonster->bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = ControlledMonster->GetCharacterMovement())
	{
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->bOrientRotationToMovement     = true;
		MoveComp->RotationRate                  = FRotator(0.f, 540.f, 0.f);
		MoveComp->bEnablePhysicsInteraction     = false;
		MoveComp->bSweepWhileNavWalking         = false;
	}

	// 감지 범위를 몬스터 설정값으로 동기화
	if (SightConfig)
	{
		SightConfig->SightRadius     = ControlledMonster->DetectionRadius;
		SightConfig->LoseSightRadius = ControlledMonster->MaxChaseDistance;
		AIPerceptionComp->ConfigureSense(*SightConfig);
		AIPerceptionComp->RequestStimuliListenerUpdate();
	}

	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
		this, &AUK_AiMonsterCtl::OnPerceptionUpdated);

	// BT 시작 및 블랙보드 초기화
	if (ControlledMonster->BehaviorTree)
	{
		UBlackboardComponent* BB = nullptr;

		UseBlackboard(ControlledMonster->BehaviorTree->BlackboardAsset,BB);

		RunBehaviorTree(ControlledMonster->BehaviorTree);


		if (BB)
		{
			BB->SetValueAsVector(TEXT("SpawnLocation"), ControlledMonster->SpawnLocation);
			BB->SetValueAsVector(TEXT("PatrolLocation"), ControlledMonster->SpawnLocation);
		}
	}
	else
	{
		ControlledMonster->RequestState(EMonsterState::Idle);
	}

	// RVO 회피 설정
	if (bUseRVOAvoidance)
	{
		if (ACharacter* Char = Cast<ACharacter>(InPawn))
		{
			if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
			{
				MoveComp->bUseRVOAvoidance             = true;
				MoveComp->SetAvoidanceGroup(AvoidanceGroup);
				MoveComp->SetGroupsToAvoid(GroupsToAvoid);
				MoveComp->SetGroupsToIgnore(GroupsToIgnore);
				MoveComp->AvoidanceConsiderationRadius = 200.0f;
				MoveComp->AvoidanceWeight              = 0.5f;
			}
		}
	}
}

void AUK_AiMonsterCtl::OnUnPossess()
{
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(
			this, &AUK_AiMonsterCtl::OnPerceptionUpdated);
	}

	ClearFocus(EAIFocusPriority::Gameplay);
	ControlledMonster = nullptr;
	CurrentTarget     = nullptr;
	bHasPatrolTarget  = false;

	Super::OnUnPossess();
}
#pragma endregion

#pragma region Player Detection
bool AUK_AiMonsterCtl::IsPlayerCharacter(AActor* Actor) const
{
	if (!Actor) return false;
	if (Cast<AUK_CharacterBase>(Actor)) return true;
	if (APawn* TestPawn = Cast<APawn>(Actor))
	{
		if (TestPawn->IsPlayerControlled()) return true;
	}
	return false;
}

void AUK_AiMonsterCtl::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!ControlledMonster || !Actor) return;
	if (!IsPlayerCharacter(Actor)) return;

	UBlackboardComponent* BB = GetBlackboardComponent();

	if (Stimulus.WasSuccessfullySensed())
	{
		CurrentTarget = Actor;

		if (BB)
		{
			if (!BB->GetValueAsObject(TEXT("TargetPlayer")))
			{
				BB->SetValueAsObject(TEXT("PendingTarget"), Actor);
			}
		}

		if (!ControlledMonster->BehaviorTree)
		{
			const float Dist = FVector::Dist(ControlledMonster->GetActorLocation(), Actor->GetActorLocation());
			ControlledMonster->RequestState(Dist <= AttackRange ? EMonsterState::Attack : EMonsterState::Chase);
		}
	}
	else
	{
		if (CurrentTarget == Actor)
		{
			CurrentTarget = nullptr;

			if (BB)
			{
				BB->ClearValue(TEXT("TargetPlayer"));
				BB->ClearValue(TEXT("PendingTarget"));
			}

			if (!ControlledMonster->BehaviorTree)
			{
				ControlledMonster->RequestState(EMonsterState::Patrol);
			}
		}
	}
}
#pragma endregion

#pragma region State Management
void AUK_AiMonsterCtl::UpdateState()
{
	if (!CurrentTarget)
	{
		ControlledMonster->RequestState(EMonsterState::Patrol);
		return;
	}

	const float Distance = FVector::Dist(
		ControlledMonster->GetActorLocation(),
		CurrentTarget->GetActorLocation());

	if (Distance <= AttackRange)
		ControlledMonster->RequestState(EMonsterState::Attack);
	else if (Distance <= ChaseRange)
		ControlledMonster->RequestState(EMonsterState::Chase);
	else
	{
		CurrentTarget = nullptr;
		ControlledMonster->RequestState(EMonsterState::Patrol);
	}
}
#pragma endregion

#pragma region Movement
void AUK_AiMonsterCtl::UpdateFocusOnTarget(AActor* NewTarget)
{
	// Focus 사용 안 함
}

void AUK_AiMonsterCtl::HandleMovement()
{
	if (!ControlledMonster) return;

	switch (ControlledMonster->GetCurrentState())
	{
	case EMonsterState::Chase:
		if (CurrentTarget) MoveToActor(CurrentTarget, AttackRange - 50.f);
		break;
	case EMonsterState::Patrol:
		if (!bHasPatrolTarget) SetNewPatrolTarget();
		MoveToLocation(PatrolTarget, 50.f);
		break;
	default:
		StopMovement();
		break;
	}
}

void AUK_AiMonsterCtl::SetNewPatrolTarget()
{
	if (!ControlledMonster) return;

	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(200.f, PatrolRadius);
	RandomOffset.Z   = 0.f;
	PatrolTarget     = ControlledMonster->SpawnLocation + RandomOffset;
	bHasPatrolTarget = true;
}

void AUK_AiMonsterCtl::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (ControlledMonster && ControlledMonster->GetCurrentState() == EMonsterState::Patrol)
	{
		bHasPatrolTarget = false;
	}
}
#pragma endregion