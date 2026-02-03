#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_Attack.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_BTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UUK_BTTask_Attack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
};
