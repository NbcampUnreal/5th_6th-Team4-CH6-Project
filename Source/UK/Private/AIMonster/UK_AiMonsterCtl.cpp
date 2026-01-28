#include "AIMonster/UK_AiMonsterCtl.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h" //디버깅용 (삭제예정)

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
		/* AI 활성화 시키기 */
		ControlledMonster->RequestState(EMonsterState::Idle);
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

	if (!ControlledMonster || !HasAuthority())
		return;

	UpdateTarget();
	UpdateState();
	HandleMovement();
	DrawAIDebug();
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

/* Patrol 범위 설정 로직 */

void AUK_AiMonsterCtl::SetNewPatrolTarget()
{
	if (!ControlledMonster)
		return;

	FVector Origin = ControlledMonster->GetActorLocation();
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(200.f, PatrolRadius);

	PatrolTarget = Origin + RandomOffset;
	bHasPatrolTarget = true;
}

/* 반복 Patrol을 할 수 있게 하는 콜백 함수 */

void AUK_AiMonsterCtl::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (ControlledMonster && ControlledMonster->GetCurrentState() == EMonsterState::Patrol)
	{
		bHasPatrolTarget = false;
	}
}

/* 디버깅용 (삭제 예정) */

void AUK_AiMonsterCtl::DrawAIDebug() const
{
	if (!bDrawDebug || !ControlledMonster)
		return;

	const FVector Origin = ControlledMonster->GetActorLocation();
	const float ZOffset = 10.f;

	// 탐색 범위 (노랑)
	DrawDebugSphere(
		GetWorld(),
		Origin,
		SearchRadius,
		32,
		FColor::Yellow,
		false,
		0.f,
		0,
		1.5f
	);

	// 추적 범위 (파랑)
	DrawDebugSphere(
		GetWorld(),
		Origin,
		ChaseRange,
		32,
		FColor::Blue,
		false,
		0.f,
		0,
		1.5f
	);

	// 공격 범위 (빨강)
	DrawDebugSphere(
		GetWorld(),
		Origin,
		AttackRange,
		32,
		FColor::Red,
		false,
		0.f,
		0,
		2.5f
	);

	// 순찰 반경 (초록)
	DrawDebugSphere(
		GetWorld(),
		Origin,
		PatrolRadius,
		32,
		FColor::Green,
		false,
		0.f,
		0,
		1.0f
	);

	// 현재 타겟 표시
	if (CurrentTarget)
	{
		DrawDebugLine(
			GetWorld(),
			Origin,
			CurrentTarget->GetActorLocation(),
			FColor::Red,
			false,
			0.f,
			0,
			2.f
		);
	}

	// 순찰 목표 지점
	if (bDrawDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			PatrolTarget,
			50.f,
			12,
			FColor::Green,
			false,
			2.f
		);
	}
}