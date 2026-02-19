#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_GetNextPatrolPoint.generated.h"

UCLASS()
class UK_API UUK_BTTask_GetNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UUK_BTTask_GetNextPatrolPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
};
