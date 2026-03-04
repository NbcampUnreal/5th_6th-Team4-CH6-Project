#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTaskNode_SetIsReturning.generated.h"

UCLASS()
class UK_API UUK_BTTaskNode_SetIsReturning : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUK_BTTaskNode_SetIsReturning();
    
protected:
	UPROPERTY(EditAnywhere, Category="Blackboard")
	bool bNewValue = true;
    
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
};
