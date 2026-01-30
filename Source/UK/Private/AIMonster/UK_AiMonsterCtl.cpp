#include "AIMonster/UK_AiMonsterCtl.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h" //������ (��������)

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
	/* ���� ��Ű�� */
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

	// BehaviorTree 사용중이면 기존 AI 로직 스킵
	if (ControlledMonster->BehaviorTree && GetBrainComponent())
		return;
	
	UpdateTarget();
	UpdateState();
	HandleMovement();
	DrawAIDebug();
}

/* Target Ž�� ���� (����,÷�� �� �� ����) */

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

/* ���º� �Ǵ� ���� */

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

/* �̵� ���� ���� */

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

/* Patrol ���� ���� ���� */

void AUK_AiMonsterCtl::SetNewPatrolTarget()
{
	if (!ControlledMonster)
		return;

	FVector Origin = ControlledMonster->GetActorLocation();
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(200.f, PatrolRadius);

	PatrolTarget = Origin + RandomOffset;
	bHasPatrolTarget = true;
}

/* �ݺ� Patrol�� �� �� �ְ� �ϴ� �ݹ� �Լ� */

void AUK_AiMonsterCtl::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (ControlledMonster && ControlledMonster->GetCurrentState() == EMonsterState::Patrol)
	{
		bHasPatrolTarget = false;
	}
}

/* ������ (���� ����) */

void AUK_AiMonsterCtl::DrawAIDebug() const
{
	if (!bDrawDebug || !ControlledMonster)
		return;

	const FVector Origin = ControlledMonster->GetActorLocation();
	const float ZOffset = 10.f;

	// Ž�� ���� (���)
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

	// ���� ���� (�Ķ�)
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

	// ���� ���� (����)
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

	// ���� �ݰ� (�ʷ�)
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

	// ���� Ÿ�� ǥ��
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

	// ���� ��ǥ ����
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