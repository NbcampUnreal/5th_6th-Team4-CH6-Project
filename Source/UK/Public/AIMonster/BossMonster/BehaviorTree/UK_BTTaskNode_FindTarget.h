#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTaskNode_FindTarget.generated.h"

UCLASS()
class UK_API UUK_BTTaskNode_FindTarget : public UBTTaskNode
{
	GENERATED_BODY()
public:
	
	UUK_BTTaskNode_FindTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
};
