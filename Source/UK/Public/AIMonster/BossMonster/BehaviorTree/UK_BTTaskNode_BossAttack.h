#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTaskNode_BossAttack.generated.h"

UCLASS()
class UK_API UUK_BTTaskNode_BossAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUK_BTTaskNode_BossAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
};
