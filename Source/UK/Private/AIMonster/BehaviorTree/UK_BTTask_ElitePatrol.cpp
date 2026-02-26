#include "AIMonster/BehaviorTree/UK_BTTask_ElitePatrol.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIMonster/Monster/UK_EliteMonster.h"
#include "GameFramework/CharacterMovementComponent.h"

UUK_BTTask_ElitePatrol::UUK_BTTask_ElitePatrol()
{
	NodeName        = "Elite Patrol";
	bNotifyTick     = true;
	bCreateNodeInstance = true;

	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_ElitePatrol, SpawnLocationKey));
}

uint16 UUK_BTTask_ElitePatrol::GetInstanceMemorySize() const
{
	return sizeof(FElitePatrolMemory);
}

EBTNodeResult::Type UUK_BTTask_ElitePatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Warning, TEXT("[ElitePatrol] ExecuteTask called"));
	
	FElitePatrolMemory* Mem = reinterpret_cast<FElitePatrolMemory*>(NodeMemory);
	*Mem = FElitePatrolMemory{};

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(AICon->GetPawn());
	if (!Elite) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	FVector TargetLocation = FVector::ZeroVector;
	bool bFoundTarget = false;

	// ── 랜덤 순찰 모드 ─────────────────────────────────────────────────
	FVector SpawnLocation = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	float Radius = Elite->PatrolRadius;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Elite->GetWorld());
	if (NavSys)
	{
		for (int32 i = 0; i < MaxNavRetries; ++i)
		{
			FNavLocation NavResult;
			if (NavSys->GetRandomPointInNavigableRadius(SpawnLocation, Radius, NavResult))
			{
				TargetLocation = NavResult.Location;
				bFoundTarget   = true;
				break;
			}
		}
	}

	if (!bFoundTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ElitePatrol] %s: Could not find patrol target"), *Elite->GetName());
		return EBTNodeResult::Failed;
	}

	Mem->TargetLocation = TargetLocation;

	if (!RequestMoveTo(AICon, TargetLocation))
	{
		return EBTNodeResult::Failed;
	}

	Mem->bMoving = true;
	
	if (!bFoundTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("[ElitePatrol] Nav target not found - SpawnLoc: %s"),
			*BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName).ToString());
		return EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress;
}

void UUK_BTTask_ElitePatrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FElitePatrolMemory* Mem = reinterpret_cast<FElitePatrolMemory*>(NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(AICon->GetPawn());
	if (!Elite) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	// ── 도착 후 대기 중 ───────────────────────────────────────────────
	if (Mem->bWaiting)
	{
		Mem->WaitTimeLeft -= DeltaSeconds;
		if (Mem->WaitTimeLeft <= 0.f)
		{
			// 대기 완료 → 다음 순찰 위치로 (태스크 재실행은 BT 가 처리)
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}

	// ── 이동 중: 도착 체크 ─────────────────────────────────────────────
	if (Mem->bMoving)
	{
		float Dist = FVector::Dist(Elite->GetActorLocation(), Mem->TargetLocation);
		if (Dist <= AcceptanceRadius)
		{
			AICon->StopMovement();
			Mem->bMoving  = false;
			Mem->bWaiting = true;
			Mem->WaitTimeLeft = FMath::RandRange(MinWaitAtWaypoint, MaxWaitAtWaypoint);

			UE_LOG(LogTemp, Log, TEXT("[ElitePatrol] %s: Arrived! Waiting %.1fs"),
				*Elite->GetName(), Mem->WaitTimeLeft);
			return;
		}

		// 스턱 감지 — 속도가 거의 없고 목적지와 멀면 재이동
		if (UCharacterMovementComponent* MC = Elite->GetCharacterMovement())
		{
			if (MC->Velocity.SizeSquared() < 100.f && Dist > AcceptanceRadius * 2.f)
			{
				++Mem->NavRetryCount;
				if (Mem->NavRetryCount <= MaxNavRetries)
				{
					RequestMoveTo(AICon, Mem->TargetLocation);
				}
				else
				{
					// 목적지 포기 → Succeeded 로 다음 포인트 진행
					UE_LOG(LogTemp, Warning, TEXT("[ElitePatrol] %s: Stuck — skipping waypoint"), *Elite->GetName());
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				}
			}
		}
	}
}

EBTNodeResult::Type UUK_BTTask_ElitePatrol::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		AICon->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

bool UUK_BTTask_ElitePatrol::RequestMoveTo(AAIController* AICon, const FVector& Dest)
{
	if (!AICon) return false;

	FAIMoveRequest MoveReq(Dest);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetAllowPartialPath(true);

	FNavPathSharedPtr NavPath;
	EPathFollowingRequestResult::Type Result = AICon->MoveTo(MoveReq, &NavPath);

	return Result != EPathFollowingRequestResult::Failed;
}