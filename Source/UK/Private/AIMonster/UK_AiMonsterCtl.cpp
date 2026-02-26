#include "AIMonster/UK_AiMonsterCtl.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/UK_CharacterBase.h"  
#include "DrawDebugHelpers.h"

AUK_AiMonsterCtl::AUK_AiMonsterCtl()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = ControllerTickInterval;

	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1200.f;
	SightConfig->LoseSightRadius = 1500.f;
	SightConfig->PeripheralVisionAngleDegrees = 360.f;
	SightConfig->SetMaxAge(5.f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AUK_AiMonsterCtl::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledMonster = Cast<AAIMonsterBase>(InPawn);
	if (!ControlledMonster) return;
	
	ControlledMonster->bUseControllerRotationYaw = false;
	
	if (UCharacterMovementComponent* MoveComp = ControlledMonster->GetCharacterMovement())
	{
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
		
		MoveComp->bEnablePhysicsInteraction = false;  
		MoveComp->bSweepWhileNavWalking = false;  
	}

	if (SightConfig)
	{
		SightConfig->SightRadius = ControlledMonster->DetectionRadius;
		SightConfig->LoseSightRadius = ControlledMonster->MaxChaseDistance;
		AIPerceptionComp->ConfigureSense(*SightConfig);
		AIPerceptionComp->RequestStimuliListenerUpdate();
	}

	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
		this, &AUK_AiMonsterCtl::OnPerceptionUpdated);

	if (ControlledMonster->BehaviorTree)
	{
		RunBehaviorTree(ControlledMonster->BehaviorTree);

		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsVector(TEXT("SpawnLocation"), ControlledMonster->SpawnLocation);
			BB->SetValueAsVector(TEXT("PatrolLocation"), ControlledMonster->SpawnLocation);
		}

		UE_LOG(LogTemp, Log, TEXT("UK_AiMonsterCtl: BT started for %s"), *InPawn->GetName());
	}
	else
	{
		ControlledMonster->RequestState(EMonsterState::Idle);
	}

	if (bUseRVOAvoidance)
	{
		if (ACharacter* Char = Cast<ACharacter>(InPawn))
		{
			if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
			{
				MoveComp->bUseRVOAvoidance = true;
				MoveComp->SetAvoidanceGroup(AvoidanceGroup);
				MoveComp->SetGroupsToAvoid(GroupsToAvoid);
				MoveComp->SetGroupsToIgnore(GroupsToIgnore);
				MoveComp->AvoidanceConsiderationRadius = 200.0f;
				MoveComp->AvoidanceWeight = 0.5f;
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
	CurrentTarget = nullptr;
	bHasPatrolTarget = false;

	Super::OnUnPossess();
}

void AUK_AiMonsterCtl::UpdateFocusOnTarget(AActor* NewTarget)
{
	// Focus 사용 안 함
}

/* ============================================================ */
/* 플레이어 판별                                                  */
/* ============================================================ */

bool AUK_AiMonsterCtl::IsPlayerCharacter(AActor* Actor) const
{
	if (!Actor) return false;

	if (Cast<AUK_CharacterBase>(Actor))
	{
		return true;
	}

	if (APawn* TestPawn = Cast<APawn>(Actor))
	{
		if (TestPawn->IsPlayerControlled())
		{
			return true;
		}
	}

	return false;
}

void AUK_AiMonsterCtl::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!ControlledMonster || !HasAuthority() || !Actor) return;

	if (!IsPlayerCharacter(Actor))
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();

	if (Stimulus.WasSuccessfullySensed())
	{
		CurrentTarget = Actor;

		if (BB)
		{
			BB->SetValueAsObject(TEXT("TargetPlayer"), Actor);
		}
		
		UE_LOG(LogTemp, Log, TEXT("[Perception] %s detected player: %s"),
			*ControlledMonster->GetName(), *Actor->GetName());

		if (!ControlledMonster->BehaviorTree)
		{
			float Dist = FVector::Dist(
				ControlledMonster->GetActorLocation(), Actor->GetActorLocation());

			if (Dist <= AttackRange)
				ControlledMonster->RequestState(EMonsterState::Attack);
			else
				ControlledMonster->RequestState(EMonsterState::Chase);
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
			}
			
			UE_LOG(LogTemp, Log, TEXT("[Perception] %s lost player: %s"),
				*ControlledMonster->GetName(), *Actor->GetName());

			if (!ControlledMonster->BehaviorTree)
			{
				ControlledMonster->RequestState(EMonsterState::Patrol);
			}
		}
	}
}

/* ============================================================ */
/* Tick                                                          */
/* ============================================================ */

void AUK_AiMonsterCtl::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITOR
	DrawAIDebug();
#endif

	if (!ControlledMonster || !HasAuthority()) return;

	if (ControlledMonster->BehaviorTree && GetBrainComponent())
	{
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			AActor* BBTarget = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetPlayer")));
			if (BBTarget != CurrentTarget)
			{
				CurrentTarget = BBTarget;
			}
		}
		return;
	}

	UpdateState();
	HandleMovement();
}

void AUK_AiMonsterCtl::UpdateState()
{
	if (!CurrentTarget)
	{
		ControlledMonster->RequestState(EMonsterState::Patrol);
		return;
	}

	float Distance = FVector::Dist(
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

void AUK_AiMonsterCtl::HandleMovement()
{
	if (!ControlledMonster) return;

	switch (ControlledMonster->GetCurrentState())
	{
	case EMonsterState::Chase:
		if (CurrentTarget)
			MoveToActor(CurrentTarget, AttackRange - 50.f);
		break;
	case EMonsterState::Patrol:
		if (!bHasPatrolTarget)
			SetNewPatrolTarget();
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

	FVector Origin = ControlledMonster->SpawnLocation;
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(200.f, PatrolRadius);
	RandomOffset.Z = 0.f;

	PatrolTarget = Origin + RandomOffset;
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

void AUK_AiMonsterCtl::DrawAIDebug() const
{
	if (!bDrawDebug || !ControlledMonster) return;

	const FVector Origin = ControlledMonster->GetActorLocation();
	const float LifeTime = ControllerTickInterval * 1.2f;

	if (CurrentTarget)
	{
		DrawDebugLine(GetWorld(), Origin, CurrentTarget->GetActorLocation(),
			FColor::Red, false, LifeTime, 0, 2.f);
	}

	if (bHasPatrolTarget)
	{
		DrawDebugSphere(GetWorld(), PatrolTarget, 50.f, 12, FColor::Green, false, LifeTime, 0, 1.5f);
	}
}