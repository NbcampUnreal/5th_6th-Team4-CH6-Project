#include "AIMonster/BehaviorTree/UK_BTTask_ElitePatrol.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIMonster/Monster/UK_EliteMonster.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
UUK_BTTask_ElitePatrol::UUK_BTTask_ElitePatrol()
{
	NodeName            = "Elite Patrol";
	bNotifyTick         = true;
	bCreateNodeInstance = true;

	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_ElitePatrol, SpawnLocationKey));
}

uint16 UUK_BTTask_ElitePatrol::GetInstanceMemorySize() const
{
	return sizeof(FElitePatrolMemory);
}
#pragma endregion

#pragma region Execution
EBTNodeResult::Type UUK_BTTask_ElitePatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FElitePatrolMemory* Mem = reinterpret_cast<FElitePatrolMemory*>(NodeMemory);
	*Mem = FElitePatrolMemory{};

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(AICon->GetPawn());
	if (!Elite) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	// ── 랜덤 순찰 위치 탐색 ─────────────────────────────────────────────
	const FVector SpawnLocation = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	FVector TargetLocation      = FVector::ZeroVector;
	bool    bFoundTarget        = false;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Elite->GetWorld());
	if (NavSys)
	{
		for (int32 i = 0; i < MaxNavRetries; ++i)
		{
			FNavLocation NavResult;
			if (NavSys->GetRandomPointInNavigableRadius(SpawnLocation, Elite->PatrolRadius, NavResult))
			{
				TargetLocation = NavResult.Location;
				bFoundTarget   = true;
				break;
			}
		}
	}

	if (!bFoundTarget) return EBTNodeResult::Failed;

	Mem->TargetLocation = TargetLocation;

	if (!RequestMoveTo(AICon, TargetLocation)) return EBTNodeResult::Failed;

	Mem->bMoving = true;
	return EBTNodeResult::InProgress;
}
#pragma endregion

#pragma region Patrol Tick
void UUK_BTTask_ElitePatrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FElitePatrolMemory* Mem = reinterpret_cast<FElitePatrolMemory*>(NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(AICon->GetPawn());
	if (!Elite) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	// ── 대기 중 ──────────────────────────────────────────────────────────
	if (Mem->bWaiting)
	{
		Mem->WaitTimeLeft -= DeltaSeconds;
		if (Mem->WaitTimeLeft <= 0.f)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}

	// ── 이동 중: 도착 체크 + 스턱 감지 ─────────────────────────────────
	if (Mem->bMoving)
	{
		const float Dist = FVector::Dist(Elite->GetActorLocation(), Mem->TargetLocation);
		if (Dist <= AcceptanceRadius)
		{
			AICon->StopMovement();
			Mem->bMoving      = false;
			Mem->bWaiting     = true;
			Mem->WaitTimeLeft = FMath::RandRange(MinWaitAtWaypoint, MaxWaitAtWaypoint);
			return;
		}

		if (UCharacterMovementComponent* MC = Elite->GetCharacterMovement())
		{
			if (MC->Velocity.SizeSquared() < 100.f && Dist > AcceptanceRadius * 2.f)
			{
				++Mem->NavRetryCount;
				if (Mem->NavRetryCount <= MaxNavRetries)
					RequestMoveTo(AICon, Mem->TargetLocation);
				else
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
	}
}
#pragma endregion

#pragma region Abort
EBTNodeResult::Type UUK_BTTask_ElitePatrol::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		AICon->StopMovement();
	}
	return EBTNodeResult::Aborted;
}
#pragma endregion

#pragma region Navigation
bool UUK_BTTask_ElitePatrol::RequestMoveTo(AAIController* AICon, const FVector& Dest)
{
	if (!AICon) return false;

	FAIMoveRequest MoveReq(Dest);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetAllowPartialPath(true);

	FNavPathSharedPtr NavPath;
	return AICon->MoveTo(MoveReq, &NavPath) != EPathFollowingRequestResult::Failed;
}
#pragma endregion