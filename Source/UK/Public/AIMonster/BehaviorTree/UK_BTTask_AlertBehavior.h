#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_AlertBehavior.generated.h"

/**
 * 경계 행동을 수행하는 Task
 * 플레이어에게서 멀어지는 행동 (뒷걸음질)
 */
UCLASS()
class UK_API UUK_BTTask_AlertBehavior : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UUK_BTTask_AlertBehavior();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	float BackOffDistance = 200.0f;  // 뒤로 물러나는 거리
};