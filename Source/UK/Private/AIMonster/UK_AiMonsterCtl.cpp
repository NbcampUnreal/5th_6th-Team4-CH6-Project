#include "AIMonster/UK_AiMonsterCtl.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h" //디버그용 드로우 (삭제예정)

AUK_AiMonsterCtl::AUK_AiMonsterCtl()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = ControllerTickInterval;
}

void AUK_AiMonsterCtl::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledMonster = Cast<AAIMonsterBase>(InPawn);

	if (ControlledMonster)
	{
		// ===== BehaviorTree =====
		if (ControlledMonster->BehaviorTree)
		{
			RunBehaviorTree(ControlledMonster->BehaviorTree);
			
			if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
			{
				BlackboardComp->SetValueAsVector(TEXT("SpawnLocation"), ControlledMonster->SpawnLocation);
				BlackboardComp->SetValueAsVector(TEXT("PatrolLocation"), ControlledMonster->SpawnLocation);
			}
			
			UE_LOG(LogTemp, Log, TEXT("UK_AiMonsterCtl: BehaviorTree started for %s"), *InPawn->GetName());
		}
		else
		{
			ControlledMonster->RequestState(EMonsterState::Idle);
		}
	
		// ===== RVO =====
		if (bUseRVOAvoidance)
		{
			ACharacter* ControlledCharacter = Cast<ACharacter>(InPawn);
			if (ControlledCharacter)
			{
				UCharacterMovementComponent* MovementComp = ControlledCharacter->GetCharacterMovement();
				if (MovementComp)
				{
					MovementComp->bUseRVOAvoidance = true;
					MovementComp->SetAvoidanceGroup(AvoidanceGroup);
					MovementComp->SetGroupsToAvoid(GroupsToAvoid);
					MovementComp->SetGroupsToIgnore(GroupsToIgnore);
					MovementComp->AvoidanceConsiderationRadius = 500.0f;
					MovementComp->AvoidanceWeight = 0.5f;
					
					UE_LOG(LogTemp, Log, TEXT("UK_AiMonsterCtl: RVO Avoidance enabled for %s"), *InPawn->GetName());
				}
			}
		}
	}
}

void AUK_AiMonsterCtl::OnUnPossess()
{
	/* 해제 시키기 */
	ControlledMonster = nullptr;
	CurrentTarget = nullptr;
	bHasPatrolTarget = false;

	Super::OnUnPossess();
}

void AUK_AiMonsterCtl::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DrawAIDebug();
	if (!ControlledMonster || !HasAuthority())
		return;

	// BehaviorTree 사용중이면 기존 AI 로직 스킵
	if (ControlledMonster->BehaviorTree && GetBrainComponent())
		return;
	
	UpdateTarget();
	UpdateState();
	HandleMovement();
	
}

/* Target 탐색 로직 (수정,첨삭 될 수 있음) */

void AUK_AiMonsterCtl::UpdateTarget()
{
	if (CurrentTarget && IsValid(CurrentTarget))
		return;

	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),ACharacter::StaticClass(),Players);

	float ClosestDist = SearchRadius;
	AActor* ClosestTarget = nullptr;

	for (AActor* Actor : Players)
	{
		if (!Actor || Actor == ControlledMonster)
			continue;

		float Dist = FVector::Dist(ControlledMonster->GetActorLocation(),Actor->GetActorLocation());

		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			ClosestTarget = Actor;
		}
	}

	CurrentTarget = ClosestTarget;
}

/* 상태별 판단 로직 */

void AUK_AiMonsterCtl::UpdateState()
{
	if (!CurrentTarget)
	{
		ControlledMonster->RequestState(EMonsterState::Patrol);
		return;
	}

	float Distance = FVector::Dist(ControlledMonster->GetActorLocation(),CurrentTarget->GetActorLocation());

	if (Distance <= AttackRange)
	{
		ControlledMonster->RequestState(EMonsterState::Attack);
	}
	else if (Distance <= ChaseRange)
	{
		ControlledMonster->RequestState(EMonsterState::Chase);
	}
	else
	{
		CurrentTarget = nullptr;
		ControlledMonster->RequestState(EMonsterState::Patrol);
	}
}

/* 이동 행위 로직 */

void AUK_AiMonsterCtl::HandleMovement()
{
	if (!ControlledMonster)
		return;

	switch (ControlledMonster->GetCurrentState())
	{
	case EMonsterState::Chase:
		if (CurrentTarget)
		{
			MoveToActor(CurrentTarget, AttackRange - 50.f);
		}
		break;

	case EMonsterState::Patrol:
		if (!bHasPatrolTarget)
		{
			SetNewPatrolTarget();
		}
		MoveToLocation(PatrolTarget, 50.f); 
		break;

	case EMonsterState::Idle:
		break;
	case EMonsterState::Attack:
		break;
	case EMonsterState::Dead:
		break;

	default:
		StopMovement();
		break;
	}
}

/* Patrol 위치 설정 */

void AUK_AiMonsterCtl::SetNewPatrolTarget()
{
	if (!ControlledMonster)
		return;

	FVector Origin = ControlledMonster->GetActorLocation();
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(200.f, PatrolRadius);

	PatrolTarget = Origin + RandomOffset;
	bHasPatrolTarget = true;
}

/* 이동 완료 콜백 */

void AUK_AiMonsterCtl::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (ControlledMonster && ControlledMonster->GetCurrentState() == EMonsterState::Patrol)
	{
		bHasPatrolTarget = false;
	}
}

/* 디버그용 드로우 (삭제예정) */

void AUK_AiMonsterCtl::DrawAIDebug() const
{
	if ( !bDrawDebug || !ControlledMonster )
		return;

	const FVector Origin = ControlledMonster->GetActorLocation();
	const float LifeTime = ControllerTickInterval * 1.2f;

	DrawDebugSphere(GetWorld(), Origin, SearchRadius, 32, FColor::Yellow, false, LifeTime, 0, 1.5f); 	// 탐색 범위 (노랑)
	DrawDebugSphere(GetWorld(), Origin, ChaseRange, 32, FColor::Blue, false, LifeTime, 0, 1.5f);		// 추적 범위 (파랑)
	DrawDebugSphere(GetWorld(), Origin, AttackRange, 32, FColor::Red, false, LifeTime, 0, 2.5f);		// 공격 범위 (빨강)
	DrawDebugSphere(GetWorld(), Origin, PatrolRadius, 32, FColor::Green, false, LifeTime, 0, 1.0f);		// 순찰 반경 (초록)

	if ( CurrentTarget ) // 현재 타겟 표시
	{
		DrawDebugLine(GetWorld(), Origin, CurrentTarget->GetActorLocation(), FColor::Red, false, LifeTime, 0, 2.f);
	}

	if ( bHasPatrolTarget )
	{
		DrawDebugSphere(GetWorld(), PatrolTarget, 50.f, 12, FColor::Green, false, LifeTime, 0, 1.5f);
	}
}