#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_BossAttackPattern.generated.h"


UCLASS()
class UK_API UUK_BTTask_BossAttackPattern : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UUK_BTTask_BossAttackPattern();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};
