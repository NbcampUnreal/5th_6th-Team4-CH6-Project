#include "AIMonster/BehaviorTree/UK_BTTask_FindRandomLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTTask_FindRandomLocation::UUK_BTTask_FindRandomLocation()
{
	NodeName = "Find Random Location";

	ResultLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_FindRandomLocation, ResultLocationKey));
	OriginLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_FindRandomLocation, OriginLocationKey));
}
#pragma endregion

#pragma region Execution
EBTNodeResult::Type UUK_BTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	UWorld* World = GetWorld();
	if (!World) return EBTNodeResult::Failed;

	// ── 탐색 중심점 결정 ─────────────────────────────────────────────────
	const FVector SearchOrigin = GetSearchOrigin(OwnerComp, Pawn);

	// ── Wander 모드면 반경 조정 ──────────────────────────────────────────
	float UsedRadius = SearchRadius;
	if (SearchType == ELocationSearchType::Wander)
	{
		UsedRadius = GetSearchRadiusForWander(Pawn);
	}

	// ── 유효한 위치 탐색 ─────────────────────────────────────────────────
	FVector ResultLocation;
	if (FindValidLocation(World, SearchOrigin, UsedRadius, MinDistanceFromOrigin, ResultLocation))
	{
		BB->SetValueAsVector(ResultLocationKey.SelectedKeyName, ResultLocation);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
#pragma endregion

#pragma region Helpers
FVector UUK_BTTask_FindRandomLocation::GetSearchOrigin(UBehaviorTreeComponent& OwnerComp, APawn* Pawn)
{
	switch (SearchType)
	{
		case ELocationSearchType::Patrol:
		case ELocationSearchType::Custom:
		{
			// Blackboard에서 OriginLocationKey 읽기
			if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
			{
				return BB->GetValueAsVector(OriginLocationKey.SelectedKeyName);
			}
			return FVector::ZeroVector;
		}

		case ELocationSearchType::Wander:
		{
			// 현재 위치 사용 (단, SpawnLocation 근처면 현재 위치 우선)
			AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Pawn);
			if (Monster && !Monster->SpawnLocation.IsNearlyZero())
			{
				const float DistFromSpawn = FVector::Dist2D(Pawn->GetActorLocation(), Monster->SpawnLocation);
				const float Threshold = SearchRadius * 0.7f;
				
				if (DistFromSpawn < Threshold)
				{
					return Pawn->GetActorLocation();
				}
				return Monster->SpawnLocation;
			}
			return Pawn->GetActorLocation();
		}

		default:
			return Pawn->GetActorLocation();
	}
}

float UUK_BTTask_FindRandomLocation::GetSearchRadiusForWander(APawn* Pawn)
{
	if (!bUseMonsterRadiusFromAsset)
	{
		return WanderRadiusOverride;
	}

	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Pawn))
	{
		return Monster->PatrolRadius * 0.5f;  // Wander는 Patrol의 절반
	}

	return SearchRadius;
}

bool UUK_BTTask_FindRandomLocation::FindValidLocation(UWorld* World, const FVector& Origin, 
                                                        float Radius, float MinDist, FVector& OutLocation)
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys) return false;

	const int32 MaxAttempts = 5;

	// ── MinDist 조건 만족하는 위치 탐색 ──────────────────────────────────
	for (int32 i = 0; i < MaxAttempts; ++i)
	{
		FNavLocation NavResult;
		if (NavSys->GetRandomPointInNavigableRadius(Origin, Radius, NavResult))
		{
			if (MinDist <= 0.f || FVector::Dist2D(Origin, NavResult.Location) >= MinDist)
			{
				OutLocation = NavResult.Location;
				return true;
			}
		}
	}

	// ── 폴백: MinDist 포기하고 아무 위치 ─────────────────────────────────
	FNavLocation FallbackResult;
	if (NavSys->GetRandomPointInNavigableRadius(Origin, Radius, FallbackResult))
	{
		OutLocation = FallbackResult.Location;
		return true;
	}

	return false;
}
#pragma endregion