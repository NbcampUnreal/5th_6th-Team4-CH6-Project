#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_FindRandomLocation.generated.h"

/**
 * FindPatrolLocation + FindWanderLocation 통합 버전
 * 
 * SearchType으로 동작 구분:
 * - Patrol: SpawnLocation 중심으로 탐색
 * - Wander: 현재 위치 중심으로 탐색
 */
UENUM(BlueprintType)
enum class ELocationSearchType : uint8
{
	Patrol      UMETA(DisplayName = "Patrol (From Spawn)"),
	Wander      UMETA(DisplayName = "Wander (From Current)"),
	Custom      UMETA(DisplayName = "Custom Origin")
};

UCLASS()
class UK_API UUK_BTTask_FindRandomLocation : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_FindRandomLocation();
#pragma endregion

#pragma region Configuration
public:
	// ── 탐색 타입 ───────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Search", meta = (DisplayName = "Search Type"))
	ELocationSearchType SearchType = ELocationSearchType::Patrol;

	// ── Blackboard Keys ──────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ResultLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard", 
	          meta = (EditCondition = "SearchType == ELocationSearchType::Patrol || SearchType == ELocationSearchType::Custom", 
	                  EditConditionHides))
	FBlackboardKeySelector OriginLocationKey;

	// ── 탐색 파라미터 ───────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "100.0"))
	float SearchRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "0.0"))
	float MinDistanceFromOrigin = 0.f;

	// ── Wander 전용 옵션 ─────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Wander", 
	          meta = (EditCondition = "SearchType == ELocationSearchType::Wander", EditConditionHides))
	bool bUseMonsterRadiusFromAsset = true;

	UPROPERTY(EditAnywhere, Category = "Wander", 
	          meta = (EditCondition = "SearchType == ELocationSearchType::Wander && !bUseMonsterRadiusFromAsset", 
	                  EditConditionHides, ClampMin = "100.0"))
	float WanderRadiusOverride = 500.f;
#pragma endregion

#pragma region Execution
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	FVector GetSearchOrigin(UBehaviorTreeComponent& OwnerComp, APawn* Pawn);
	float GetSearchRadiusForWander(APawn* Pawn);
	bool FindValidLocation(UWorld* World, const FVector& Origin, float Radius, 
	                       float MinDist, FVector& OutLocation);
#pragma endregion
};