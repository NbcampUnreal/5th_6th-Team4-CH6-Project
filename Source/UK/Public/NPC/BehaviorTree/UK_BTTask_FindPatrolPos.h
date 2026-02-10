
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_FindPatrolPos.generated.h"


UCLASS()
class UK_API UUK_BTTask_FindPatrolPos : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UUK_BTTask_FindPatrolPos();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolRadius = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolLocationKey;
};
