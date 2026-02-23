// Fill out your copyright notice in the Description page of Project Settings.

#include "AIMonster/BehaviorTree/UK_BTTask_FindWanderLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTTask_FindWanderLocation::UUK_BTTask_FindWanderLocation()
{
	NodeName = "Find Wander Location";
	WanderLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_FindWanderLocation, WanderLocationKey));
}

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

	const float DistFromSpawn = FVector::Dist2D(ControlledPawn->GetActorLocation(), SearchOrigin);
	if (DistFromSpawn < UsedRadius * 0.7f)
	{
		SearchOrigin = ControlledPawn->GetActorLocation();
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(OwnerComp.GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;

	// 폰의 실제 Z값 — NavMesh Z 대신 이걸로 고정해서 공중 뜨는 현상 방지
	const float PawnZ = ControlledPawn->GetActorLocation().Z;

	const int32 MaxAttempts = 5;
	const float MinWanderDistance = 150.0f;

	for (int32 i = 0; i < MaxAttempts; ++i)
	{
		FNavLocation ResultLocation;
		if (NavSystem->GetRandomPointInNavigableRadius(SearchOrigin, UsedRadius, ResultLocation))
		{
			const float DistToCurrent = FVector::Dist2D(ControlledPawn->GetActorLocation(), ResultLocation.Location);
			if (DistToCurrent >= MinWanderDistance)
			{
				FVector FinalLocation = ResultLocation.Location;
				FinalLocation.Z = PawnZ;

				BlackboardComp->SetValueAsVector(WanderLocationKey.SelectedKeyName, FinalLocation);
				return EBTNodeResult::Succeeded;
			}
		}
	}

	// 최소 거리 포기하고 폴백
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