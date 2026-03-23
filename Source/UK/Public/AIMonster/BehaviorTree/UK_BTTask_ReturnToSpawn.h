#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_ReturnToSpawn.generated.h"

/**
 * 스폰 위치로 복귀하는 태스크
 */
UCLASS()
class UK_API UUK_BTTask_ReturnToSpawn : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UUK_BTTask_ReturnToSpawn();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Return")
	float ArrivalDistance = 100.f;

	UPROPERTY(EditAnywhere, Category = "Return")
	float ReturnSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Return")
	float StuckRetryInterval = 1.0f;

private:
	void RestoreSpeed(APawn* InPawn);

	float OriginalMaxWalkSpeed = 0.f;
	bool  bSpeedBoosted = false;
	float StuckRetryTimer = 0.f;
};