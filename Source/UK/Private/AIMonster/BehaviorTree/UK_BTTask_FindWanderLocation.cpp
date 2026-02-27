#include "AIMonster/BehaviorTree/UK_BTTask_FindWanderLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTTask_FindWanderLocation::UUK_BTTask_FindWanderLocation()
{
	NodeName = "Find Wander Location";
	WanderLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_FindWanderLocation, WanderLocationKey));
}
#pragma endregion

#pragma region Wander Location Search
EBTNodeResult::Type UUK_BTTask_FindWanderLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);

	float UsedRadius = WanderRadius;
	if (bUseMonsterRadiusFromAsset && Monster)
	{
		UsedRadius = Monster->PatrolRadius * 0.5f;
	}

	FVector SearchOrigin = (Monster && !Monster->SpawnLocation.IsNearlyZero())
		? Monster->SpawnLocation
		: ControlledPawn->GetActorLocation();

	// 스폰 가까이 있으면 현재 위치를 중심으로 배회
	if (FVector::Dist2D(ControlledPawn->GetActorLocation(), SearchOrigin) < UsedRadius * 0.7f)
	{
		SearchOrigin = ControlledPawn->GetActorLocation();
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(OwnerComp.GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;

	const float PawnZ            = ControlledPawn->GetActorLocation().Z;
	const int32 MaxAttempts      = 5;
	const float MinWanderDist    = 150.0f;

	// ── 최소 거리 조건을 만족하는 위치 탐색 ─────────────────────────────
	for (int32 i = 0; i < MaxAttempts; ++i)
	{
		FNavLocation ResultLocation;
		if (NavSystem->GetRandomPointInNavigableRadius(SearchOrigin, UsedRadius, ResultLocation))
		{
			if (FVector::Dist2D(ControlledPawn->GetActorLocation(), ResultLocation.Location) >= MinWanderDist)
			{
				BlackboardComp->SetValueAsVector(WanderLocationKey.SelectedKeyName, ResultLocation.Location);
				return EBTNodeResult::Succeeded;
			}
		}
	}

	// ── 폴백: 최소 거리 포기 ─────────────────────────────────────────────
	FNavLocation FallbackLocation;
	if (NavSystem->GetRandomPointInNavigableRadius(SearchOrigin, UsedRadius, FallbackLocation))
	{
		FVector FinalLocation = FallbackLocation.Location;
		FinalLocation.Z = PawnZ;
		BlackboardComp->SetValueAsVector(WanderLocationKey.SelectedKeyName, FinalLocation);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
#pragma endregion