#include "AIMonster/UK_AiMonsterCtl.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "DrawDebugHelpers.h"

AUK_AiMonsterCtl::AUK_AiMonsterCtl()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = ControllerTickInterval;

	// ===== AIPerception 설정 =====
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1200.f;
	SightConfig->LoseSightRadius = 1500.f;
	SightConfig->PeripheralVisionAngleDegrees = 360.f;  // 전방위
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

	// Perception 반경을 몬스터 설정에 맞게 조정
	if (SightConfig)
	{
		SightConfig->SightRadius = ControlledMonster->DetectionRadius;
		SightConfig->LoseSightRadius = ControlledMonster->MaxChaseDistance;
		AIPerceptionComp->ConfigureSense(*SightConfig);
		AIPerceptionComp->RequestStimuliListenerUpdate();
	}

	// Perception 이벤트 바인딩
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
		this, &AUK_AiMonsterCtl::OnPerceptionUpdated);

	// ===== BehaviorTree =====
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

	// ===== RVO =====
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
				MoveComp->AvoidanceConsiderationRadius = 500.0f;
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

	ControlledMonster = nullptr;
	CurrentTarget = nullptr;
	bHasPatrolTarget = false;

	Super::OnUnPossess();
}

/* ============================================================ */
/* AIPerception 이벤트 콜백                                      */
/* 타겟 감지/소실 시에만 호출 (매 틱 X)                            */
/* ============================================================ */

void AUK_AiMonsterCtl::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!ControlledMonster || !HasAuthority() || !Actor) return;

	// 몬스터끼리는 무시
	if (Cast<AAIMonsterBase>(Actor)) return;

	UBlackboardComponent* BB = GetBlackboardComponent();

	if (Stimulus.WasSuccessfullySensed())
	{
		// 타겟 감지됨
		CurrentTarget = Actor;

		if (BB)
		{
			BB->SetValueAsObject(TEXT("TargetPlayer"), Actor);
		}

		UE_LOG(LogTemp, Log, TEXT("[Perception] %s detected: %s"),
			*ControlledMonster->GetName(), *Actor->GetName());

		// BT 미사용 시 상태 전환
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
		// 타겟 소실됨
		if (CurrentTarget == Actor)
		{
			CurrentTarget = nullptr;

			if (BB)
			{
				BB->ClearValue(TEXT("TargetPlayer"));
			}

			UE_LOG(LogTemp, Log, TEXT("[Perception] %s lost: %s"),
				*ControlledMonster->GetName(), *Actor->GetName());

			if (!ControlledMonster->BehaviorTree)
			{
				ControlledMonster->RequestState(EMonsterState::Patrol);
			}
		}
	}
}

/* ============================================================ */
/* Tick — BT 사용 시 디버그만, 미사용 시 Fallback                 */
/* ============================================================ */

void AUK_AiMonsterCtl::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITOR
	DrawAIDebug();
#endif

	if (!ControlledMonster || !HasAuthority()) return;

	// BT 사용 중이면 Tick AI 로직 스킵
	if (ControlledMonster->BehaviorTree && GetBrainComponent())
		return;

	// Fallback: BT 미사용 시만
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

	FVector Origin = ControlledMonster->GetActorLocation();
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

	DrawDebugSphere(GetWorld(), Origin, SearchRadius, 24, FColor::Yellow, false, LifeTime, 0, 1.0f);
	DrawDebugSphere(GetWorld(), Origin, ChaseRange, 24, FColor::Blue, false, LifeTime, 0, 1.0f);
	DrawDebugSphere(GetWorld(), Origin, AttackRange, 24, FColor::Red, false, LifeTime, 0, 1.5f);

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