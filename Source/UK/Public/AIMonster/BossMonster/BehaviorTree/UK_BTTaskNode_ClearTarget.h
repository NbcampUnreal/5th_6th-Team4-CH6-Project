#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTaskNode_ClearTarget.generated.h"

UCLASS()
class UK_API UUK_BTTaskNode_ClearTarget : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUK_BTTaskNode_ClearTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
};
