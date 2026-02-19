#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_ReturnToSpawn.generated.h"

/**
 * 스폰 복귀 Task (자체 이동 + 속도 부스트)
 * 
 * 스폰에서 멀면: 속도 부스트 → 스폰으로 이동 → 도착 시 속도 복원 → Succeeded
 * 스폰 근처면: Failed (다음 브랜치로)
 * 
 * BT에서 이 Task 하나만 배치하면 됨 (별도 MoveTo 불필요)
 */
UCLASS()
class UK_API UUK_BTTask_ReturnToSpawn : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UUK_BTTask_ReturnToSpawn();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	// 복귀 시 속도 배율 (기본 이동속도 × 이 값)
	UPROPERTY(EditAnywhere, Category = "AI")
	float ReturnSpeedMultiplier = 5.0f;

	// 스폰 도착 판정 거리
	UPROPERTY(EditAnywhere, Category = "AI")
	float ArrivalDistance = 200.0f;

private:
	float OriginalMaxWalkSpeed = 0.f;
	bool bSpeedBoosted = false;

	void RestoreSpeed(APawn* InPawn);
};