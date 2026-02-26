#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_CheckResetDistance.generated.h"

/**
 * 적과의 거리를 체크해서 리셋 거리를 벗어났는지 확인하는 Task
 */
UCLASS()
class UK_API UUK_BTTask_CheckResetDistance : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_CheckResetDistance();
#pragma endregion

#pragma region Reset Distance Check
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Blackboard Keys
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;
#pragma endregion
};