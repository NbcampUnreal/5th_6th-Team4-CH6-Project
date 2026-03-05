#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_ElitePatrol.generated.h"

/**
 * 엘리트 몬스터 순찰 태스크
 * ■ 랜덤 순찰 모드 : 스폰 위치 기준 PatrolRadius 내 랜덤 이동.
 */
UCLASS()
class UK_API UUK_BTTask_ElitePatrol : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_ElitePatrol();
	virtual uint16 GetInstanceMemorySize() const override;
#pragma endregion

#pragma region Execution
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Patrol Tick
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
#pragma endregion

#pragma region Abort
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Blackboard Keys
	/** 스폰 위치 블랙보드 키 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;
#pragma endregion

#pragma region Patrol Settings
	/** 도착 판정 거리 */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float AcceptanceRadius = 100.0f;

	/** 웨이포인트 도착 후 대기 시간 범위 (초) */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float MinWaitAtWaypoint = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float MaxWaitAtWaypoint = 4.0f;

	/** 랜덤 순찰 모드 — 이동 실패 시 최대 재시도 횟수 */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	int32 MaxNavRetries = 3;
#pragma endregion

#pragma region Memory
private:
	struct FElitePatrolMemory
	{
		bool    bMoving        = false;
		bool    bWaiting       = false;
		float   WaitTimeLeft   = 0.f;
		FVector TargetLocation = FVector::ZeroVector;
		int32   NavRetryCount  = 0;
	};
#pragma endregion

#pragma region Navigation
	bool RequestMoveTo(AAIController* AICon, const FVector& Dest);
#pragma endregion
};