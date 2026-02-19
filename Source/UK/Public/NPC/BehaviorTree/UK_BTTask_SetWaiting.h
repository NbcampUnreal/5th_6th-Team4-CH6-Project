
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_SetWaiting.generated.h"

UCLASS()
class UK_API UUK_BTTask_SetWaiting : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UUK_BTTask_SetWaiting();

	UPROPERTY(EditAnywhere)
	bool bWaiting;

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};
